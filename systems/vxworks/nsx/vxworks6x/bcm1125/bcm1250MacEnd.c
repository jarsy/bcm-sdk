/* bcm1250MacEnd.c - END style BCM1250 MAC Ethernet driver */

/* Copyright 2002-2004 Wind River Systems, Inc. */

#include "copyright_wrs.h"

/* $Id: bcm1250MacEnd.c,v 1.3 2011/07/21 16:14:48 yshtil Exp $
 * Copyright 2000,2001
 * Broadcom Corporation. All rights reserved.
 *
 * This software is furnished under license to Wind River Systems, Inc.
 * and may be used only in accordance with the terms and conditions of
 * this license.  No title or ownership is transferred hereby.
 */

/*
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01l,22sep05,wap  Correct previous SPR fix (SPR #112722)
01k,13sep05,wap  Correct clSize adjustment (SPR #112176)
01j,23jun04,agf  make address space conversion macros mapped-memory compatible
01j,05jun04,dgp  clean up formatting to correct doc build errors
01i,30jan03,m_h  IPv6 Support
01h,26jun02,pgh  Fix SPR 79037, polled mode.
01g,21jun02,agf  change comment block to match coding standard
01f,19mar02,pgh  Apply code review fixes.
01e,14mar02,pgh  Made the code compliant with the coding standard.
                 Eliminated unused code.
                 Added comments.
                 Restructured areas of the code.
01d,06mar02,pgh  Fixes SPR 73552 and SPR 74000.
01c,05mar02,pgh  Fixes SPR73549, SPR73550, and SPR73644.
01b,07dec01,agf  apply coding standard fix-ups
01a,15nov01,agf  written.
*/

/*
DESCRIPTION
This module implements the Broadcom BCM1250 on-chip ethernet MACs.
The BCM1250 ethernet DMA has two channels, but this module only 
supports channel 0.  The dual DMA channel feature is intended for 
packet classification and quality of service applications.

EXTERNAL INTERFACE
The only external interface is the bcm1250MacEndLoad() routine, which has the
<initString> as its only parameter.  The initString parameter must be a colon-
delimited string in the following format:

<unit>:<hwunit>:<vecnum>:<flags>:<numRds0>:<numTds0>

TARGET-SPECIFIC PARAMETERS
\is
\i <unit>
This parameter defines which ethernet interface is being loaded.

\i <hwunit>
This parameter is no longer used, but must be present so the string can be 
parsed properly.  Its value should be zero.

\i <vecnum>
This parameter specifies the interrupt vector number.  This driver configures 
the MAC device to generate hardware interrupts for various events within the 
device; thus it contains an interrupt handler routine.  The driver calls 
bcm1250IntConnect() to connect its interrupt handler to this interrupt vector.

\i <flags>
Device specific flags, for future use.  Its value should be zero.

\i <numRds0>
This parameter specifies the number of receive DMA buffer descriptors for DMA
channel 0.

\i <numTds0>
This parameter specifies the number of transmit DMA buffer descriptors for DMA
channel 0.
\ie

SYSTEM RESOURCE USAGE
When implemented, this driver requires the following system resources:

    - one mutual exclusion semaphore
    - one interrupt vector
    - 68 bytes in the initialized data section (data)
    - 0 bytes in the uninitialized data section (BSS)

    The driver allocates clusters of size 1520 bytes for receive frames and
    and transmit frames.

INCLUDES:
endLib.h etherMultiLib.h bcm1250MacEnd.h

SEE ALSO: muxLib, endLib, netBufLib
\tb "Writing and Enhanced Network Driver"

*/

#include "vxWorks.h"
#include "stdio.h"
#include "stdlib.h"
#include "logLib.h"
#include "semLib.h"
#include "intLib.h"
#include "netLib.h"
#include "netBufLib.h"
#include "memLib.h"
#include "etherLib.h"
#include "etherMultiLib.h"
#include "endLib.h"
#include "lstLib.h"
#ifdef WR_IPV6
#include "adv_net.h"
#endif /*WR_IPV6*/

#include "bcm1250Lib.h"
#include "config.h"
#include "bcm1250MacEnd.h"


#define DRV_DEBUG

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

#define DRV_LOG(FLG, X0, X1, X2, X3, X4, X5, X6)                        \
        if (bcm1250MacDebug & FLG)                                             \
            (void)logMsg (X0, X1, X2, X3, X4, X5, X6);
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


#define KVTOPHYS(x) ((UINT32)(x) & 0x1FFFFFFF)
#define PHYSTOV(x)  ((UINT32)(x) | 0x80000000)

#ifndef ETH_MAC_REG_READ
#define ETH_MAC_REG_READ(reg)  \
    MIPS3_LD (pDrvCtrl->regMacBase + reg)
#endif /* ETH_MAC_REG_READ */

#ifndef ETH_MAC_REG_WRITE
#define ETH_MAC_REG_WRITE(reg, val)  \
    MIPS3_SD ((pDrvCtrl->regMacBase + reg), (val))
#endif /* ETH_MAC_REG_WRITE */


#ifndef ETH_DMA_REG_READ
#define ETH_DMA_REG_READ(reg)   MIPS3_LD (reg)
#endif /* ETH_DMA_REG_READ */

#ifndef ETH_DMA_REG_WRITE
#define ETH_DMA_REG_WRITE(reg, val)     MIPS3_SD ((reg), (val))
#endif /* ETH_DMA_REG_WRITE */


#define NET_BUF_ALLOC() \
    netClusterGet (pDrvCtrl->endObj.pNetPool, pDrvCtrl->clPoolId)

#define NET_BUF_FREE(pBuf) \
    netClFree (pDrvCtrl->endObj.pNetPool, (unsigned char *)(pBuf))

#define NET_MBLK_ALLOC() \
    mBlkGet (pDrvCtrl->endObj.pNetPool, M_DONTWAIT, MT_DATA)

#define NET_MBLK_FREE(pMblk) \
    netMblkFree (pDrvCtrl->endObj.pNetPool, (M_BLK_ID)pMblk)

#define NET_CL_BLK_ALLOC() \
    clBlkGet (pDrvCtrl->endObj.pNetPool, M_DONTWAIT)

#define NET_CL_BLK_FREE(pClblk) \
    clBlkFree (pDrvCtrl->endObj.pNetPool, (CL_BLK_ID)pClBlk)

#define NET_MBLK_BUF_FREE(pMblk) \
    (void)netMblkClFree ((M_BLK_ID)pMblk)

#define NET_MBLK_CL_JOIN(pMblk, pClBlk) \
    (void)netMblkClJoin ((pMblk), (pClBlk))

#define NET_CL_BLK_JOIN(pClBlk, pBuf, len) \
    (void)netClBlkJoin ((pClBlk), (pBuf), (len), (FUNCPTR)NULL, 0, 0, 0)

/* PHY/MII */

#define MII_COMMAND_START       0x01
#define MII_COMMAND_READ        0x02
#define MII_COMMAND_WRITE       0x01
#define MII_COMMAND_ACK         0x02

/* Basic Mode Control Register bit definitions */

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
#define BMCR_SPEED1000 (BMCR_SPEED1 | BMCR_SPEED0)
#define BMCR_SPEED100  (BMCR_SPEED0)
#define BMCR_SPEED10    0

/* Basic Mode Status Register bit definitions */

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

/* 1K Status Register bit definitions */

#define K1STSR_MSMCFLT  0x8000
#define K1STSR_MSCFGRES 0x4000
#define K1STSR_LRSTAT   0x2000
#define K1STSR_RRSTAT   0x1000
#define K1STSR_LP1KFD   0x0800
#define K1STSR_LP1KHD   0x0400
#define K1STSR_LPASMDIR 0x0200

/* AutoNegotiation Link Partner Abilities Register bit definitions */

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

/* Physical interface chip register index definitions */

#define MII_BMCR    0x00    /* Basic mode control register (rw) */
#define MII_BMSR    0x01    /* Basic mode status register (ro) */
#define MII_K1STSR  0x0A    /* 1K Status Register (ro) */
#define MII_ANLPAR  0x05    /* AutoNegotiation Link Partner Abilities (rw) */

#define M_MAC_MDIO_DIR_OUTPUT   0               /* for clarity */

/* externs */

IMPORT void sysBcm1250MacEnetAddrGet (int, char *);
IMPORT STATUS bcm1250IntConnect (int, int, VOIDFUNCPTR, int);
IMPORT STATUS bcm1250IntDisconnect (int);
IMPORT STATUS bcm1250IntEnable (int);
IMPORT STATUS bcm1250IntDisable (int);


/* locals */

#ifdef  DRV_DEBUG
/*
LOCAL int bcm1250MacDebug = DRV_DEBUG_LOAD | DRV_DEBUG_INT | DRV_DEBUG_TX |
                            DRV_DEBUG_RX | DRV_DEBUG_POLL;
*/

LOCAL int bcm1250MacDebug = DRV_DEBUG_OFF;
#endif /*DRV_DEBUG*/

char *  pTxPollBuf;     /* Points to polled mode transmit cluster */

/* forward declarations */

void bcm1250MacRxDmaShow (int);    /* for debug */
void bcm1250MacTxDmaShow (int);    /* for debug */
void bcm1250MacShow (int);         /* for debug */
void bcm1250MacPhyShow (int);      /* for debug */

LOCAL UINT64 bcm1250MacAddr2Reg (unsigned char *);
LOCAL STATUS bcm1250MacMemInit (DRV_CTRL *);
LOCAL STATUS bcm1250MacDmaInit (ETH_MAC_DMA *, MAC_REG, int);
LOCAL STATUS bcm1250MacPktCopyTransmit (DRV_CTRL *, M_BLK *);
LOCAL STATUS bcm1250MacPktTransmit (DRV_CTRL *, M_BLK *, int);
LOCAL void bcm1250MacTxHandle (DRV_CTRL *);
LOCAL void bcm1250MacRxHandle (DRV_CTRL *);
LOCAL STATUS bcm1250MacInitParse (DRV_CTRL *, char *);
LOCAL void bcm1250MacInt (DRV_CTRL *);
LOCAL void bcm1250MacRxFilterSet (DRV_CTRL *);
LOCAL void  bcm1250MacMblkWalk (M_BLK *, int *, BOOL *);
LOCAL unsigned bcm1250MacEthHash (unsigned char *);
LOCAL void bcm1250MacEthMiiPoll (DRV_CTRL *);
LOCAL void bcm1250MacEthMiiFindPhy (DRV_CTRL *);
LOCAL void bcm1250MacEthMiiSync (DRV_CTRL *);
LOCAL void bcm1250MacEthMiiSendData (DRV_CTRL *, unsigned int, int);
LOCAL int bcm1250MacEthMiiRead (DRV_CTRL *, int, int);
LOCAL STATUS bcm1250MacHashRegAdd (DRV_CTRL *, char *);
LOCAL STATUS bcm1250MacHashRegSet (DRV_CTRL *);
LOCAL STATUS bcm1250MacSetConfig (DRV_CTRL *, MAC_SPEED, MAC_DUPLEX, MAC_FC);

/* This is the only externally visible interface. */

END_OBJ * bcm1250MacEndLoad (char * initString);

/* END Specific interfaces. */

LOCAL STATUS bcm1250MacStart (DRV_CTRL *);
LOCAL STATUS bcm1250MacStop (DRV_CTRL *);
LOCAL STATUS bcm1250MacUnload (DRV_CTRL *);
LOCAL int bcm1250MacIoctl (DRV_CTRL *, int, caddr_t);
LOCAL STATUS bcm1250MacSend (DRV_CTRL *, M_BLK_ID);
LOCAL STATUS bcm1250MacMCastAdd (DRV_CTRL *, char *);
LOCAL STATUS bcm1250MacMCastDel (DRV_CTRL *, char *);
LOCAL STATUS bcm1250MacMCastGet (DRV_CTRL *, MULTI_TABLE *);
LOCAL STATUS bcm1250MacPollSend (DRV_CTRL *, M_BLK_ID);
LOCAL STATUS bcm1250MacPollRcv (DRV_CTRL *, M_BLK_ID);
LOCAL void bcm1250MacPollStart (DRV_CTRL *);
LOCAL void bcm1250MacPollStop (DRV_CTRL *);


/*
 * Define the device function table.  This is static across all driver
 * instances.
 */

LOCAL NET_FUNCS bcm1250MacFuncTable =
    {
    (FUNCPTR)bcm1250MacStart,       /* Function to start the device. */
    (FUNCPTR)bcm1250MacStop,        /* Function to stop the device. */
    (FUNCPTR)bcm1250MacUnload,      /* Unloading function for the driver. */
    (FUNCPTR)bcm1250MacIoctl,       /* Ioctl function for the driver. */
    (FUNCPTR)bcm1250MacSend,        /* Send function for the driver. */

    (FUNCPTR)bcm1250MacMCastAdd,    /* Multicast add function for the */
    (FUNCPTR)bcm1250MacMCastDel,    /* Multicast delete function for */
    (FUNCPTR)bcm1250MacMCastGet,    /* Multicast retrieve function for */

    (FUNCPTR)bcm1250MacPollSend,    /* Polling send function */
    (FUNCPTR)bcm1250MacPollRcv,     /* Polling receive function */

    endEtherAddressForm,            /* put address info into a NET_BUFFER */
    (FUNCPTR)endEtherPacketDataGet, /* get pointer to data in NET_BUFFER */
    (FUNCPTR)endEtherPacketAddrGet  /* Get packet addresses. */
    };


/*******************************************************************************
*
* bcm1250MacEndLoad - initialize the driver and device
*
* This routine initializes the driver and the device to the operational state.
* All of the device specific parameters are passed in <initString>, which
* expects a string of the following format:
*
* The initialization string format is:
* "<unit>:<hwunit>:<vecnum>:<flags>:<numRds0>:<numTds0>:<numRds1>:<numTds1>"
*
* The hwunit field is not used, but must be present to parse properly.
*
* This routine can be called in two modes.  If it is called with an empty but
* allocated string, it places the name of this device (that is, "sbe0", "sbe1",
* or "sbe2") into the <initString> and returns NULL.
*
* If the string is allocated and not empty, the routine attempts to load
* the driver using the values specified in the string.
*
* RETURNS: An END object pointer or NULL on error.
*/

END_OBJ * bcm1250MacEndLoad
    (
    char *  initString            /* String to be parsed by the driver. */
    )
    {
    STATUS      rtv;
    DRV_CTRL *  pDrvCtrl;   /* driver control structure */
    char        enetAddr[6];

    DRV_LOG (DRV_DEBUG_LOAD, "Loading bcm1250MacEnd...\n", 1, 2, 3, 4, 5, 6);

    /* Check for an allocated string. */

    if (initString == (char *)NULL)
        {
        DRV_LOG (DRV_DEBUG_LOAD, "bcm1250MacEndLoad: NULL initStr\n",
                 1, 2, 3, 4, 5, 6);
        return ((END_OBJ *)NULL);
        }

    /* If a null string, then copy the device's name to the string. */

    if (initString[0] == '\0')
        {
        bcopy ((char *)SB_DEV_NAME, initString, SB_DEV_NAME_LEN);
        DRV_LOG (DRV_DEBUG_LOAD, "bcm1250MacEndLoad: initString[0]==NUL\n",
                 1, 2, 3, 4, 5, 6);
        return ((END_OBJ *)NULL);
        }

    /* Allocate the device structure */

    pDrvCtrl = (DRV_CTRL *)calloc (sizeof (DRV_CTRL), 1);
    if (pDrvCtrl == (DRV_CTRL *)NULL)
        {
        DRV_LOG (DRV_DEBUG_LOAD,
                 "bcm1250MacEndLoad: alloc for pDrvCtrl failed.\n",
                 1, 2, 3, 4, 5, 6);
        return ((END_OBJ *)NULL);
        }

    /* Parse the init string, filling in the device structure */

    if (bcm1250MacInitParse (pDrvCtrl, initString) == ERROR)
        {
        DRV_LOG (DRV_DEBUG_LOAD, "bcm1250MacEndLoad: init parse failed\n",
                 1, 2, 3, 4, 5, 6);
        goto errorExit;
        }

    /* Initialize register addresses. */

    pDrvCtrl->regMacEnable = R_MAC_ENABLE;
    pDrvCtrl->regMacCfg    = R_MAC_CFG;
    pDrvCtrl->regFifoCfg   = R_MAC_THRSH_CFG;
    pDrvCtrl->regFrameCfg  = R_MAC_FRAMECFG;
    pDrvCtrl->regRxFilter  = R_MAC_ADFILTER_CFG;
    pDrvCtrl->regIsr       = R_MAC_STATUS;
    pDrvCtrl->regImr       = R_MAC_INT_MASK;
    pDrvCtrl->regMdio      = R_MAC_MDIO;

    /* Ask the BSP to provide the ethernet address. */

    sysBcm1250MacEnetAddrGet (pDrvCtrl->unit, &enetAddr[0]);

    DRV_LOG (DRV_DEBUG_LOAD,
             "Loading bcm1250MacEnd: mac addr = %x:%x:%x:%x:%x:%x\n",
             enetAddr[0], enetAddr[1], enetAddr[2], enetAddr[3], enetAddr[4],
             enetAddr[5]);

    /* initialize the END_OBJ. */

    rtv = END_OBJ_INIT (&pDrvCtrl->endObj, 
                        (DEV_OBJ *)pDrvCtrl, 
                        SB_DEV_NAME,
                        pDrvCtrl->unit, 
                        &bcm1250MacFuncTable, 
                        "BCM1250 END Driver.");
    if (rtv == ERROR)
        goto errorExit;

    /* Reset the ethernet MAC. */

    ETH_MAC_REG_WRITE (pDrvCtrl->regMacEnable, M_MAC_PORT_RESET);
    ETH_MAC_REG_WRITE (pDrvCtrl->regMacEnable, 0);

    /* Perform memory allocation/initialization for ethernet DMA receive structure. */

    DRV_LOG (DRV_DEBUG_LOAD, "rxDMA ch#0 init\n", 1, 2, 3, 4, 5, 6);
    rtv = bcm1250MacDmaInit (&pDrvCtrl->rxDma, pDrvCtrl->regDmaBase, 0);
    if (rtv == ERROR)
        goto errorExit;

    /* Perform memory allocation/initialization for ethernet DMA transmit structure. */

    DRV_LOG (DRV_DEBUG_LOAD, "txDMA ch#0 init\n", 1, 2, 3, 4, 5, 6);
    rtv = bcm1250MacDmaInit (&pDrvCtrl->txDma, pDrvCtrl->regDmaBase, 1);
    if (rtv == ERROR)
        goto errorExit;

    /* 
     * Perform memory allocation/initialization for the netBufLib-managed 
     * memory pool.
     */ 

    if (bcm1250MacMemInit (pDrvCtrl) == ERROR)
        goto errorExit;

    /* Allocate a cluster buffer for polled mode transmit. */

    pTxPollBuf = NET_BUF_ALLOC ();

    /* Find the address of the physical interface chip. */

    bcm1250MacEthMiiFindPhy (pDrvCtrl);

    /* Determine the speed and configuration of the physical interface. */

    bcm1250MacEthMiiPoll (pDrvCtrl);

    /* Initialize the MIB-II structure */

    rtv = END_MIB_INIT (&pDrvCtrl->endObj, 
                        M2_ifType_ethernet_csmacd,
                        (UINT8 *)&enetAddr[0], 
                        6, 
                        ETHERMTU,
                        BCM1250_MAC_SPEED_DEF);
    if (rtv == ERROR)
        {
        DRV_LOG (DRV_DEBUG_LOAD,
                 "bcm1250MacEndLoad: MIB init failed\n",
                 1, 2, 3, 4, 5, 6);
        goto errorExit;
        }

    /* set the flags to indicate readiness */

    END_OBJ_READY (&pDrvCtrl->endObj,
                   IFF_UP | IFF_RUNNING | IFF_NOTRAILERS | 
                   IFF_BROADCAST | IFF_MULTICAST);
    DRV_LOG (DRV_DEBUG_LOAD, "Done loading ...", 1, 2, 3, 4, 5, 6);

    return (&pDrvCtrl->endObj);

errorExit:
    free ((char *)pDrvCtrl);

    return ((END_OBJ *)NULL);
    }

/*******************************************************************************
*
* bcm1250MacInitParse - parse the initialization string
*
* Parse the input string.  This routine is called from bcm1250MacEndLoad()
* which initializes some values in the driver control structure with the
* values passed in the intialization string.
*
* The initialization string format is:
* "<unit>:<hwunit>:<vecnum>:<flags>:<numRds0>:<numTds0>"
* \is
* \i <unit>
* Device unit number, 0 - 2.
* \i <hwunit>
* Not used, but must be present to parse properly.
* \i <vecnum>
* Interrupt vector number.
* \i <flags>
* Device specific flags, for future use.
* \i <numRds0>
* Number of receive DMA buffer descriptors for channel 0.
* \i <numTds0>
* Number of transmit DMA buffer descriptors for channel 0.
* \ie
*
* RETURNS: OK, or ERROR if any arguments are invalid.
*/

LOCAL STATUS bcm1250MacInitParse
    (
    DRV_CTRL *  pDrvCtrl,   /* driver control structure */
    char *      initString
    )
    {
    char *          tok;    /* an initString token */
    char *          holder = (char *)NULL;  /* points beyond tok */
    ETH_MAC_DMA *   pRxDma0 = &pDrvCtrl->rxDma;
    ETH_MAC_DMA *   pTxDma0 = &pDrvCtrl->txDma;

    DRV_LOG (DRV_DEBUG_LOAD, "InitParse: Initstr=%s\n",
             (int)initString, 2, 3, 4, 5, 6);

    /* Process the device unit number. */

    tok = strtok_r (initString, ":", &holder);
    if (tok == (char *)NULL)
        return ERROR;
    pDrvCtrl->unit = atoi (tok);

    /* Validate the device unit number, and set the interrupt source. */

    switch (pDrvCtrl->unit)
        {
        case 0:
            pDrvCtrl->intSource = K_INT_MAC_0;
            break;
        case 1:
            pDrvCtrl->intSource = K_INT_MAC_1;
            break;
        case 2:
            pDrvCtrl->intSource = K_INT_MAC_2;
            break;
        default:
            return ERROR;
        }

    /* Discard the hwunit, which is not used. */

    tok = strtok_r ((char *)NULL, ":", &holder);
    if (tok == (char *)NULL)
        return ERROR;

    /* Set the base address for the ethernet registers for this device. */

    pDrvCtrl->regMacBase = PHYS_TO_K1 (A_MAC_CHANNEL_BASE (pDrvCtrl->unit));
    pDrvCtrl->regDmaBase = pDrvCtrl->regMacBase;

    /* Process the interrupt vector number. */

    tok = strtok_r ((char *)NULL, ":", &holder);
    if (tok == (char *)NULL)
        return ERROR;
    pDrvCtrl->iVecNum = strtoul (tok, (char **)NULL, 10);

    /* Process the Device specific flags. */

    tok = strtok_r ((char *)NULL, ":", &holder);
    if (tok == (char *)NULL)
        return (ERROR);
    pDrvCtrl->flags = strtoul (tok, (char **)NULL, 16);

    /* Process the number of receive DMA buffer descriptors for channel 0. */

    tok = strtok_r ((char *)NULL, ":", &holder);
    if (tok == (char *)NULL)
        return ERROR;
    if (atoi (tok) < 0)
        pRxDma0->maxDescr = NUM_RDS_DEF;
    else
        pRxDma0->maxDescr = atoi (tok);

    /* Process the number of transmit DMA buffer descriptors for channel 0. */

    tok = strtok_r ((char *)NULL, ":", &holder);
    if (tok == (char *)NULL)
        return ERROR;
    if (atoi (tok) < 0)
        pTxDma0->maxDescr = NUM_TDS_DEF;
    else
        pTxDma0->maxDescr = atoi (tok);

    DRV_LOG (DRV_DEBUG_LOAD,
             "EndLoad: flags=0x%x dmaBase=0x%x macBase=0x%x iVecNum=0x%x intSource=0x%x ",
             pDrvCtrl->flags, pDrvCtrl->regDmaBase, pDrvCtrl->regMacBase,
             pDrvCtrl->iVecNum, pDrvCtrl->intSource, 6);

    DRV_LOG (DRV_DEBUG_LOAD,
             "EndLoad: numRd0 %d numTd0 %d unit %d\n",
             pRxDma0->maxDescr, pTxDma0->maxDescr, pDrvCtrl->unit, 4, 5, 6);

    return OK;
    }

/*******************************************************************************
*
* bcm1250MacMemInit - initialize memory
*
* Allocates and initializes the mBlk/clBlk/cluster memory pool.
*
* RETURNS: OK or ERROR.
*/

LOCAL STATUS bcm1250MacMemInit
    (
    DRV_CTRL *  pDrvCtrl    /* driver control structure */
    )
    {
    STATUS          rtv;            /* function return value */
    M_CL_CONFIG     mClBlkConfig;   /* mBlk/clBlk configuration table */
    CL_DESC         clDesc;         /* cluster descriptor table */

    DRV_LOG (DRV_DEBUG_LOAD, "sbe%d - bcm1250MacMemInit() start.\n",
             pDrvCtrl->unit, 2, 3, 4, 5, 6);

    /*
     * Clear the mBlk/clBlk configuration table, 
     * and the cluster descriptor table.
     */

    bzero ((char *)&mClBlkConfig, sizeof (mClBlkConfig));
    bzero ((char *)&clDesc, sizeof (clDesc));

    /* Calculate the number of clusters. */

    clDesc.clNum = pDrvCtrl->rxDma.maxDescr +
                   pDrvCtrl->txDma.maxDescr +
                   (RXDSCR_LOAN_NUM * 2) + 2;

    /* Set the size of the clusters. */

    clDesc.clSize = ROUND_UP ((MAX_FRAME_SIZE + CACHELINESIZE),
                              CACHELINESIZE);

    /* Calculate the memory size for the clusters. */

    clDesc.memSize = (clDesc.clNum * (clDesc.clSize + sizeof (long))) +
                     CACHELINESIZE;

    /* Allocate memory for the clusters from cache safe memory. */

    pDrvCtrl->bufBase = (char *)memalign (CACHELINESIZE, clDesc.memSize);
    if (pDrvCtrl->bufBase == (char *)NULL)
        {
        DRV_LOG (DRV_DEBUG_LOAD,
                 "sbe%d - pDrvCtrl->bufBase alloc failed for size 0x%8x\n",
                 pDrvCtrl->unit, clDesc.memSize, 3, 4, 5, 6);
        return (ERROR);
        }

    /* Set the base address of the cluster memory. */

    clDesc.memArea = (char *)pDrvCtrl->bufBase;

    /* 
     * Purposely misalign the base address of the cluster memory by 
     * (CACHELINESIZE - sizeof (long)), and adjust the size of the 
     * clusters, so that the guts of the buffers are cache aligned.
     */

    clDesc.memArea += (CACHELINESIZE - sizeof (long));
    clDesc.clSize -= sizeof (long);
    clDesc.memSize -= (CACHELINESIZE - sizeof (long));

    DRV_LOG (DRV_DEBUG_LOAD,
             "sbe%d - InitMem NetBufLib Cluster memArea 0x%08x\n",
             pDrvCtrl->unit, (UINT32)clDesc.memArea, 3, 4, 5, 6);

    /* Set twice as many mBlk's as clusters. */

    mClBlkConfig.mBlkNum = clDesc.clNum * 2;

    /* Set the number of clusters. */

    mClBlkConfig.clBlkNum = clDesc.clNum;

    /* Calculate the memory size needed. */

    mClBlkConfig.memSize = (mClBlkConfig.mBlkNum * (M_BLK_SZ + sizeof (long))) +
                           (mClBlkConfig.clBlkNum * CL_BLK_SZ);

    /* Allocated the required memory */

    mClBlkConfig.memArea = (char *)memalign (sizeof (long),
                                             mClBlkConfig.memSize);
    if (mClBlkConfig.memArea == (char *)NULL)
        {
        DRV_LOG (DRV_DEBUG_LOAD,
                 "sbe%d - mClBlkConfig.memArea alloc failed for size 0x%8x\n",
                 pDrvCtrl->unit, mClBlkConfig.memSize, 3, 4, 5, 6);
        return (ERROR);
        }
    pDrvCtrl->mClBlkBase = mClBlkConfig.memArea;

    /* Allocate the network pool. */

    pDrvCtrl->endObj.pNetPool = (NET_POOL_ID)malloc (sizeof (NET_POOL));
    if (pDrvCtrl->endObj.pNetPool == (NET_POOL_ID)NULL)
        {
        DRV_LOG (DRV_DEBUG_LOAD,
                 "sbe%d - pNetPool alloc failed for size 0x%8x\n",
                 pDrvCtrl->unit, sizeof (NET_POOL), 3, 4, 5, 6);
        return (ERROR);
        }

    /* Initialize the netBufLib-managed memory pool */

    rtv = netPoolInit (pDrvCtrl->endObj.pNetPool, &mClBlkConfig, &clDesc, 1,
                       (POOL_FUNC *)NULL);
    if (rtv == ERROR)
        {
        DRV_LOG (DRV_DEBUG_LOAD, "netPoolInit() failed.\n",
                1, 2, 3, 4, 5, 6);
        return (ERROR);
        }

    /* Get the cluster pool ID for the clusters. */

    pDrvCtrl->clPoolId = netClPoolIdGet (pDrvCtrl->endObj.pNetPool,
                                         MAX_FRAME_SIZE, FALSE);
    if (pDrvCtrl->clPoolId == (CL_POOL_ID)NULL)
        {
        DRV_LOG (DRV_DEBUG_LOAD, "netClPoolIdGet() returned NULL pool ID.\n",
                1, 2, 3, 4, 5, 6);
        return (ERROR);
        }

    DRV_LOG (DRV_DEBUG_LOAD, "sbe%d - bcm1250MacMemInit() complete\n",
             pDrvCtrl->unit, 2, 3, 4, 5, 6);

    return OK;
    }

/*******************************************************************************
*
* bcm1250MacDmaInit - initialize DMA data structure and hardware registers
*
* This routine initializes the DMA data structure.  It allocates and
* initializes memory for the DMA buffer descriptors, the buffer pointer
* table, and the buffer type table.  The buffer type table is only used
* in the transmit direction.  The variables used to manage the ring of
* buffers and descriptors are also initialized in this routine.  Finally,
* the address of the DMA registers are initialized, and the config and buffer
* descriptor base registers are initialized.
*
* RETURNS: OK, or ERROR
*/

LOCAL STATUS bcm1250MacDmaInit
    (
    ETH_MAC_DMA *   pDma,       /* ethernet DMA struct to be initialized */
    MAC_REG         dmaBaseAdr, /* base address of DMA registers */
    int             xmit        /* 1 if transmit direction, 0 for receive */
    )
    {
    int     dscr;       /* DMA buffer descriptor index */

    DRV_LOG (DRV_DEBUG_LOAD, "DMA init start\n", 1, 2, 3, 4, 5, 6);

    /* allocate memory for DMA buffer descriptor rings */

    pDma->pDscrTable = (ETH_DMA_DSCR *)memalign (CACHELINESIZE,
                                                 pDma->maxDescr *
                                                     sizeof (ETH_DMA_DSCR));
    if (pDma->pDscrTable == (ETH_DMA_DSCR *)NULL)
        {
        DRV_LOG (DRV_DEBUG_LOAD, "pDscrTable alloc failed\n", 1, 2, 3, 4, 5, 6);
        return (ERROR);
        }
    bzero ((char *)pDma->pDscrTable, (pDma->maxDescr * sizeof (ETH_DMA_DSCR)));

    /* 
     * For transmit only, allocate and initialize a table of pointers to buffers 
     * and a table of buffers types. 
     */

    if (xmit == 1)
        {
        pDma->bufTable = (TX_BUF_TABLE *)malloc (pDma->maxDescr * 
                                                 sizeof (TX_BUF_TABLE));
        if (pDma->bufTable == (TX_BUF_TABLE *)NULL)
            {
            DRV_LOG (DRV_DEBUG_LOAD, 
                     "bufTable alloc failed.\n", 1, 2, 3, 4, 5, 6);
            return (ERROR);
            }

        for (dscr = 0; dscr < pDma->maxDescr; dscr++)
            {
            pDma->bufTable[dscr].pBuf = (char *)NULL;
            pDma->bufTable[dscr].type = BUF_TYPE_NONE;
            }
        }

    /* Initialize ring management variables */

    pDma->tailIndex = 0;
    pDma->headIndex = 0;
    pDma->ringCount = 0;

    /* Transmitter is not blocked */

    pDma->txBlocked = FALSE;

    /* Initialize register addresses */

    pDma->regConfig0 = R_MAC_DMA_REGISTER (xmit, 0, R_MAC_DMA_CONFIG0) + 
                       dmaBaseAdr;
    pDma->regConfig1 = R_MAC_DMA_REGISTER (xmit, 0, R_MAC_DMA_CONFIG1) + 
                       dmaBaseAdr;
    pDma->regDscrBase = R_MAC_DMA_REGISTER (xmit, 0, R_MAC_DMA_DSCR_BASE) + 
                        dmaBaseAdr;
    pDma->regDscrCnt = R_MAC_DMA_REGISTER (xmit, 0, R_MAC_DMA_DSCR_CNT) + 
                       dmaBaseAdr;
    pDma->regCurDscr = R_MAC_DMA_REGISTER (xmit, 0, R_MAC_DMA_CUR_DSCRADDR) +
                       dmaBaseAdr;

    DRV_LOG (DRV_DEBUG_LOAD, "set regs, dscrbase and config0/maxdscr\n",
             1, 2, 3, 4, 5, 6);

    /* Initialize config registers and buffer descriptor base register */

    ETH_DMA_REG_WRITE (pDma->regConfig1, 0);
    ETH_DMA_REG_WRITE (pDma->regDscrBase, 
                       (UINT32)(KVTOPHYS (pDma->pDscrTable)));
    ETH_DMA_REG_WRITE (pDma->regConfig0, V_DMA_RINGSZ (pDma->maxDescr));

    DRV_LOG (DRV_DEBUG_LOAD, "DMA init complete\n", 1, 2, 3, 4, 5, 6);
    return (OK);
    }

/*******************************************************************************
*
* bcm1250MacUnload - unload a driver from the system
*
* This routine first brings down the device, and then frees any memory
* allocated by the driver in the load function. The controller structure
* should be freed by the calling function.
*
* RETURNS: OK, always.
*/

LOCAL STATUS bcm1250MacUnload
    (
    DRV_CTRL *  pDrvCtrl    /* driver control structure */
    )
    {
    ETH_MAC_DMA *   pDma;   /* ethernet DMA structure */

    DRV_LOG (DRV_DEBUG_LOAD, "EndUnload ...\n", 1, 2, 3, 4, 5, 6);

    /* deallocate lists */

    END_OBJ_UNLOAD (&pDrvCtrl->endObj);

    pDma = &pDrvCtrl->rxDma;

    /* Free the RX DMA buffer descriptors. */

    if (pDma->pDscrTable)
        free (pDma->pDscrTable);

    pDma = &pDrvCtrl->txDma;

    /* Free the TX buffer table. */

    if (pDma->bufTable)
        free (pDma->bufTable);

    /* Free the TX DMA buffer descriptors. */

    if (pDma->pDscrTable)
        free (pDma->pDscrTable);

    /* Free the mBlk/clBlk memory. */
    
    if (pDrvCtrl->mClBlkBase)
        free (pDrvCtrl->mClBlkBase);

    /* Free the cluster buffer memory. */

    if (pDrvCtrl->bufBase)
        free (pDrvCtrl->bufBase);

    /* Free the memory allocated for driver pool structure */

    if (pDrvCtrl->endObj.pNetPool != (NET_POOL *)NULL)
        free (pDrvCtrl->endObj.pNetPool);

    return (OK);
    }


/*******************************************************************************
*
* bcm1250MacStart - start the device
*
* This function calls BSP functions to connect interrupts and start the
* device running in interrupt mode.
*
* RETURNS: OK or ERROR
*/

LOCAL STATUS bcm1250MacStart
    (
    DRV_CTRL *  pDrvCtrl /* driver control structure */
    )
    {
    UINT64          macAddr;        /* 64 bit ethernet MAC address */
    STATUS          rtv;            /* function return value */
    int             idx;
    MAC_REG         port;
    char            enetAddr[6];    /* 6 byte ethernet MAC address */
    ETH_MAC_DMA *   pDma;           /* ethernet DMA structure */
    int             ix;
    char *          pBuf = (char *)NULL;
    ETH_DMA_DSCR *  pDscr;          /* DMA buffer descriptor */

    DRV_LOG (DRV_DEBUG_IOCTL, "bcm1250MacStart enter ......\n", 
             1, 2, 3, 4, 5, 6);

    /* reset */

    ETH_MAC_REG_WRITE (pDrvCtrl->regMacEnable, 0);

    /* get mac addr */

    sysBcm1250MacEnetAddrGet (pDrvCtrl->unit, &enetAddr[0]);
    macAddr =  bcm1250MacAddr2Reg ((unsigned char *)enetAddr);
    DRV_LOG (DRV_DEBUG_IOCTL, "macAddr=0x%08x%08x\n", 
             (int)(macAddr >> 32), macAddr, 3, 4, 5, 6);

    /* set its own hw addr */

    ETH_MAC_REG_WRITE (R_MAC_ETHERNET_ADDR, 0);

    /* clear hash table */

    port = R_MAC_HASH_BASE;
    for (idx = 0; idx < MAC_HASH_COUNT; idx++)
        {
        ETH_MAC_REG_WRITE (port, 0);
        port += sizeof (UINT64);
        }

    /* clear exact match addr table */

    port = R_MAC_ADDR_BASE;
    for (idx = 0; idx < MAC_ADDR_COUNT; idx++)
        {
        ETH_MAC_REG_WRITE (port, 0);
        port += sizeof (UINT64);
        }

    /* always interested in packets addressed to self */

    ETH_MAC_REG_WRITE (R_MAC_ADDR_BASE, macAddr);

    /* reset channel map register to only use channel 0!!! */

    port = R_MAC_CHUP0_BASE;
    for (idx = 0; idx < MAC_CHMAP_COUNT; idx++)
        {
        ETH_MAC_REG_WRITE (port, 0);
        port += sizeof (UINT64);
        }

    port = R_MAC_CHLO0_BASE;
    for (idx = 0; idx < MAC_CHMAP_COUNT; idx++)
        {
        ETH_MAC_REG_WRITE (port, 0);
        port += sizeof (UINT64);
        }

    /* clear rx filter */

    ETH_MAC_REG_WRITE (pDrvCtrl->regRxFilter, 0);

    /* no interrupt yet */

    ETH_MAC_REG_WRITE (pDrvCtrl->regImr, 0);

    /* setup frame cfg */

    ETH_MAC_REG_WRITE (pDrvCtrl->regFrameCfg,
                       V_MAC_MIN_FRAMESZ_DEFAULT |
                       V_MAC_MAX_FRAMESZ_DEFAULT |
                       V_MAC_BACKOFF_SEL (1) );

    /* setup fifo cfg */

    ETH_MAC_REG_WRITE (pDrvCtrl->regFifoCfg,
                       V_MAC_TX_WR_THRSH (4) |       /* Must be '4' or '8' */
                       V_MAC_TX_RD_THRSH (4) |
                       V_MAC_TX_RL_THRSH (4) |
                       V_MAC_RX_PL_THRSH (4) |
                       V_MAC_RX_RD_THRSH (4) |       /* Must be '4' */
                       V_MAC_RX_PL_THRSH (4) |
                       V_MAC_RX_RL_THRSH (8));

    /* setup the main config reg */

    ETH_MAC_REG_WRITE (pDrvCtrl->regMacCfg,
                       M_MAC_RETRY_EN |
                       M_MAC_TX_HOLD_SOP_EN |
                       V_MAC_TX_PAUSE_CNT_16K |
                       V_MAC_SPEED_SEL_100MBPS |
                       M_MAC_AP_STAT_EN |
                       M_MAC_FAST_SYNC |
                       M_MAC_SS_EN |
                       V_MAC_IPHDR_OFFSET (14));

    /* Set interface's speed, duplex, and flow control */

    (void)bcm1250MacSetConfig (pDrvCtrl, pDrvCtrl->macSpeed, 
                               pDrvCtrl->macDuplex, pDrvCtrl->macFc);

    /* init rx dma's descrs */

    DRV_LOG (DRV_DEBUG_LOAD, "rxDMA ch#0 init\n", 1, 2, 3, 4, 5, 6);
    pDma = &(pDrvCtrl->rxDma);

    /* Initialize all receive DMA buffer descriptors. */

    for (ix = 0; ix < pDma->maxDescr; ix++)
        {
        /* Allocate a cluster */

        pBuf = (char *)NET_BUF_ALLOC ();
        if (pBuf == (char *)NULL)
            {
            DRV_LOG (DRV_DEBUG_LOAD, "bcm1250Mac - netClusterGet failed\n",
                         1, 2, 3, 4, 5, 6);
            return (ERROR);
            }

        /* point to next buffer descriptor. */

        pDscr = &pDma->pDscrTable[pDma->tailIndex];

        /* initialize descriptor to receive into the cluster */

        pDscr->dscr_a = (UINT64)((UINT32)KVTOPHYS (pBuf)) |
                        V_DMA_DSCRA_A_SIZE (MAX_FRAME_CACHE_BLKS) |
                        M_DMA_DSCRA_INTERRUPT | VXW_RCV_BUF_OFFSET;
        pDscr->dscr_b = 0;

        /* give desc to the hw */

        ETH_DMA_REG_WRITE (pDma->regDscrCnt, 1);

        /* Advance ring management variables */

        pDma->tailIndex = (pDma->tailIndex + 1) % pDma->maxDescr;
        pDma->ringCount++;

        DRV_LOG (DRV_DEBUG_LOAD, "set reg, dscrcnt\n", 1, 2, 3, 4, 5, 6);
        DRV_LOG (DRV_DEBUG_LOAD, "descA manual read 0x%08x%08x\n",
                 (pDscr->dscr_a) >> 32, pDscr->dscr_a, 3, 4, 5, 6);
        DRV_LOG (DRV_DEBUG_LOAD, "descB manual read 0x%08x%08x\n",
                 (pDscr->dscr_b) >> 32, pDscr->dscr_b, 3, 4, 5, 6);
        }

    /* enable transmit and receive interrupts for channel 0 */

    ETH_MAC_REG_WRITE (pDrvCtrl->regMacEnable,
                       M_MAC_RXDMA_EN0 |
                       M_MAC_TXDMA_EN0 |
                       M_MAC_RX_ENABLE |
                       M_MAC_TX_ENABLE);

    DRV_LOG (DRV_DEBUG_IOCTL, "bcm1250MacStart: enabled\n", 1, 2, 3, 4, 5, 6);

    /* connect interrupt handler */

    rtv = bcm1250IntConnect (pDrvCtrl->intSource, pDrvCtrl->iVecNum,
                             bcm1250MacInt, (int) pDrvCtrl);
    if (rtv == ERROR)
        return (ERROR);
    DRV_LOG (DRV_DEBUG_IOCTL, "bcm1250MacStart connect int\n",
             1, 2, 3, 4, 5, 6);

    /* unmask the mac interrupt */

    ETH_MAC_REG_WRITE (pDrvCtrl->regImr,
                       (M_MAC_INT_CHANNEL << S_MAC_TX_CH0) |
                       (M_MAC_INT_CHANNEL << S_MAC_RX_CH0));
    DRV_LOG (DRV_DEBUG_IOCTL, "bcm1250MacStart, set intr mask\n",
             1, 2, 3, 4, 5, 6);

    /* set rxfilter */

    ETH_MAC_REG_WRITE (pDrvCtrl->regRxFilter, M_MAC_UCAST_EN | M_MAC_BCAST_EN);

    /* set flags to indicate interface is up */

    END_FLAGS_SET (&pDrvCtrl->endObj, (IFF_UP | IFF_RUNNING));
    DRV_LOG (DRV_DEBUG_IOCTL, "bcm1250MacStart, interface up\n",
             1, 2, 3, 4, 5, 6);

    /* enable interrupts */

    (void)bcm1250IntEnable (pDrvCtrl->intSource);
    DRV_LOG (DRV_DEBUG_IOCTL, "bcm1250MacStart: enable interrupt\n",
             1, 2, 3, 4, 5, 6);

    DRV_LOG (DRV_DEBUG_IOCTL, "bcm1250MacStart done\n", 1, 2, 3, 4, 5, 6);

    return OK;
    }


/*******************************************************************************
*
* bcm1250MacStop - stop the device
*
* This function calls BSP functions to disconnect interrupts and stop
* the device from operating in interrupt mode.
*
* RETURNS: OK.
*/

LOCAL STATUS bcm1250MacStop
    (
    DRV_CTRL *  pDrvCtrl    /* driver control structure */
    )
    {
    DRV_LOG (DRV_DEBUG_LOAD, "bcm1250MacStop enter ......\n", 1, 2, 3, 4, 5, 6);

    /* mark the interface as down */

    END_FLAGS_CLR (&pDrvCtrl->endObj, (IFF_UP | IFF_RUNNING));

    /* disable mac interrupts */

    ETH_MAC_REG_WRITE (pDrvCtrl->regImr, 0);

    /* disable mac hw */

    ETH_MAC_REG_WRITE (pDrvCtrl->regMacEnable, 0);

    /* diasble VxW interrupt */

    (void)bcm1250IntDisable (pDrvCtrl->intSource);

    /* disconnect interrupt handler */

    (void)bcm1250IntDisconnect (pDrvCtrl->intSource);

    DRV_LOG (DRV_DEBUG_LOAD, "bcm1250MacStop done.\n", 1, 2, 3, 4, 5, 6);

    return (OK);
    }


/*******************************************************************************
*
* bcm1250MacSend - send an Ethernet packet
*
* This routine takes a M_BLK_ID and sends off the data in the M_BLK_ID.
* The buffer must already have the addressing information properly installed
* in it. This is done by a higher layer.
*
* muxSend() calls this routine each time it wants to send a packet.
*
* RETURNS: OK, or END_ERR_BLOCK, if no resources are available, or ERROR, if
* the device is currently working in polling mode, or is passed a bad M_BLK
* pointer.
*/

LOCAL STATUS bcm1250MacSend
    (
    DRV_CTRL *  pDrvCtrl,       /* driver control structure */
    M_BLK *     pMblk           /* pointer to the mBlk/cluster pair */
    )
    {
    STATUS  rtv = OK;       /* function return value */
    int     fragNum = 0;    /* number of fragments in this mBlk */
    BOOL    zeroCopyReady;  /* able to transmit without buffer copy */

    DRV_LOG (DRV_DEBUG_TX, "bcm1250MacSend...\n", 1, 2, 3, 4, 5, 6);

    /*
     * Obtain exclusive access to transmitter.  This is necessary because
     * we might have more than one stack transmitting at once.
     */

    END_TX_SEM_TAKE (&pDrvCtrl->endObj, WAIT_FOREVER);

    /* check device mode */

    if (DRV_FLAGS_ISSET (BCM1250_MAC_POLLING))
        {
        netMblkClChainFree (pMblk);
        rtv = ERROR;
        errno = EINVAL;
        goto bcm1250MacSendError;
        }

    /* check for valid M_BLK */

    if (pMblk == (M_BLK *)NULL)
        {
        DRV_LOG (DRV_DEBUG_TX, "Invalid pMblk\n", 1, 2, 3, 4, 5, 6);
        rtv = ERROR;
        errno = EINVAL;
        goto bcm1250MacSendError;
        }

    /* walk the mBlk */

    bcm1250MacMblkWalk (pMblk, &fragNum, &zeroCopyReady);

    /* check we have enough resources */

    if (zeroCopyReady &&
        ((pDrvCtrl->txDma.maxDescr - pDrvCtrl->txDma.ringCount) >= fragNum))
        {
        DRV_LOG (DRV_DEBUG_TX, "bcm1250MacSend fragNum = %d\n", fragNum,
                 2, 3, 4, 5, 6);

        /* transmit the packet in zero-copy mode */

        rtv = bcm1250MacPktTransmit (pDrvCtrl, pMblk, fragNum);
        }
    else
        {
        /* transmit the packet in copy mode */

        rtv = bcm1250MacPktCopyTransmit (pDrvCtrl, pMblk);
        }

bcm1250MacSendError:
    END_TX_SEM_GIVE (&pDrvCtrl->endObj);

    DRV_LOG (DRV_DEBUG_TX, "bcm1250MacSend...Done\n", 1, 2, 3, 4, 5, 6);

    return (rtv);
    }

/*******************************************************************************
*
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
    DRV_CTRL *  pDrvCtrl,   /* driver control structure */
    M_BLK *     pMblk       /* pointer to the mBlk */
    )
    {
    int             len = 0;                /* length of data to be sent */
    char *          pBuf = (char *)NULL;    /* pointer to data to be sent */
    ETH_DMA_DSCR *  pTxDscr;                /* DMA buffer descriptor */
    ETH_MAC_DMA *   pTxDma;                 /* ethernet DMA structure */
    UINT64          dscrCnt;                /* buffer descriptor count */

    DRV_LOG (DRV_DEBUG_TX, "bcm1250Mac CopyTx ...\n", 1, 2, 3, 4, 5, 6);

    pTxDma = &pDrvCtrl->txDma;

    /* get a cluster buffer from the pool */

    pBuf = NET_BUF_ALLOC ();
    if (pBuf == (char *)NULL)
        {
        /* set to stall condition */

        pDrvCtrl->txDma.txBlocked = TRUE;
        return (END_ERR_BLOCK);
        }

    /* Check for all DMA buffer descriptors in use. */

    if (pTxDma->ringCount >= pTxDma->maxDescr)
        {
        DRV_LOG (DRV_DEBUG_TX, "No available TxBufs\n", 1, 2, 3, 4, 5, 6);
        END_ERR_ADD (&pDrvCtrl->endObj, MIB2_OUT_ERRS, 1);

        NET_BUF_FREE (pBuf);

        pDrvCtrl->txDma.txBlocked = TRUE;
        return (END_ERR_BLOCK); /* just return without freeing mBlk chain */
        }

    len = netMblkToBufCopy (pMblk, (char *)pBuf, (FUNCPTR)NULL);
    netMblkClChainFree (pMblk);
    len = max (ETHERSMALL, len);

    DRV_LOG (DRV_DEBUG_TXD,
             "tx - *pBuf= 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
             pBuf[0], pBuf[1], pBuf[2], pBuf[3], pBuf[4], pBuf[5]);
    DRV_LOG (DRV_DEBUG_TXD,
             "tx - *pBuf= 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
             pBuf[6], pBuf[7], pBuf[8], pBuf[9], pBuf[10], pBuf[11]);
    DRV_LOG (DRV_DEBUG_TXD,
             "tx - *pBuf= 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
             pBuf[12], pBuf[13], pBuf[14], pBuf[15], pBuf[16], pBuf[17]);

    /* point to next buffer descriptor */

    pTxDscr = &pTxDma->pDscrTable[pTxDma->tailIndex];

    DRV_LOG (DRV_DEBUG_TXD, "pTxd->desc_a = 0x%08x\n",
             (UINT32)&pTxDscr->dscr_a, 2, 3, 4, 5, 6);
    DRV_LOG (DRV_DEBUG_TXD, "pTxd->desc_b = 0x%08x\n",
             (UINT32)&pTxDscr->dscr_b, 2, 3, 4, 5, 6);

    /* set buffer ptr, size in cache line, enable intr, mark as 1st pkt */

    pTxDscr->dscr_a = (UINT64)((UINT32)KVTOPHYS (pBuf)) |
                      V_DMA_DSCRA_A_SIZE (NUMCACHEBLKS (len)) |
                      M_DMA_DSCRA_INTERRUPT |
                      M_DMA_ETHTX_SOP;

    /* set pkt len and  option */

    pTxDscr->dscr_b = V_DMA_DSCRB_OPTIONS (K_DMA_ETHTX_APPENDCRC_APPENDPAD) |
                      V_DMA_DSCRB_PKT_SIZE (len);

    DRV_LOG (DRV_DEBUG_TXD, "dscr_a = 0x%08x%08x\n", (pTxDscr->dscr_a) >> 32,
             pTxDscr->dscr_a, 3, 4, 5, 6);
    DRV_LOG (DRV_DEBUG_TXD, "dscr_b = 0x%08x%08x\n", (pTxDscr->dscr_b) >> 32,
             pTxDscr->dscr_b, 3, 4, 5, 6);

    /* save pointer to buffer and the buffer type. */

    pTxDma->bufTable[pTxDma->tailIndex].pBuf = pBuf;
    pTxDma->bufTable[pTxDma->tailIndex].type = BUF_TYPE_CL;

    /* advance ring management variables */

    pTxDma->tailIndex = (pTxDma->tailIndex + 1) % pTxDma->maxDescr;
    pTxDma->ringCount++;

    /* tell hw one more dscr to go */

    ETH_DMA_REG_WRITE (pTxDma->regDscrCnt, 1);

    /* debug purpose */

    dscrCnt = ETH_DMA_REG_READ (pTxDma->regDscrCnt);

    DRV_LOG (DRV_DEBUG_TX, "tx0 DMA dscrcnt=0x%08x%08x\n", (dscrCnt >> 32), 
             dscrCnt, 3, 4, 5, 6);

    DRV_LOG (DRV_DEBUG_TX, "bcm1250Mac CopyTx Done.\n", 1, 2, 3, 4, 5, 6);

    return (OK);
    }


/*******************************************************************************
*
* bcm1250MacMblkWalk - walk the mBlk
*
* This routine walks the mBlk whose address is in <pMblk>, computes
* the number of fragments it is made of, and returns it in the parameter
* <pFragNum>. In addition, it finds out whether the specified packet
* is unicast or multicast.
*
* RETURNS: OK, or ERROR in case of invalid mBlk.
*/

LOCAL void bcm1250MacMblkWalk
    (
    M_BLK * pMblk,          /* pointer to the mBlk */
    int *   pFragNum,       /* number of fragments in this mBlk */
    BOOL *  pZeroCopyReady  /* no buffer copy flag */
    )
    {
    M_BLK * pCurr = pMblk;  /* the current mBlk */

    DRV_LOG (DRV_DEBUG_TX, "bcm1250MacMblkWalk: start\n", 1, 2, 3, 4, 5, 6);

    *pZeroCopyReady = TRUE; /* assume no buffer copy is required */

    /* walk this mBlk */

    do
        {
        /* if not first buffer in a packet, must start at cache line boundary */

        if (pCurr != pMblk)
            {
            if (((UINT32)pCurr->mBlkHdr.mData & (CACHELINESIZE - 1)) != 0)
                {
                DRV_LOG (DRV_DEBUG_TX,
                         "bcm1250MacMblkWalk: beginning align fail\n",
                         1, 2, 3, 4, 5, 6);
                *pZeroCopyReady = FALSE;    /* buffer copy required */
                }
            }

        /* if not last buffer in a pacet, must end at cache line boundary */

        if (pCurr->mBlkHdr.mNext != (M_BLK *)NULL)
            {
            if ((((UINT32)pCurr->mBlkHdr.mData + pCurr->mBlkHdr.mLen) &
                 (CACHELINESIZE - 1)) != 0)
                {
                DRV_LOG (DRV_DEBUG_TX,
                         "bcm1250MacMblkWalk: ending align fail\n",
                         1, 2, 3, 4, 5, 6);
                *pZeroCopyReady = FALSE;    /* buffer copy required */
                }
            }

        /* keep track of the number of fragments in this mBlk */

        (*pFragNum)++;

        pCurr = ((M_BLK *)pCurr->mBlkHdr.mNext);
        DRV_LOG (DRV_DEBUG_TX, "bcm1250MacMblkWalk: next\n", 1, 2, 3, 4, 5, 6);
        }
    while (pCurr != (M_BLK *)NULL);
    }


/*******************************************************************************
*
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
    DRV_CTRL *  pDrvCtrl,   /* driver control structure */
    M_BLK *     pMblk,      /* pointer to the mBlk */
    int         fragNum     /* number of fragments in this mBlk */
    )
    {
    int             len = 0;                /* length of data to be sent */
    UINT32          offset;                 /* offset to cache line */
    char *          pBuf = (char *)NULL;    /* pointer to data to be sent */
    ETH_DMA_DSCR *  pTxDscr;                /* DMA buffer descriptor */
    ETH_MAC_DMA *   pTxDma;                 /* ethernet DMA structure */
    M_BLK *         pCurrentMblk;           /* current mBlk pointer */

    DRV_LOG (DRV_DEBUG_TX, "bcm1250Mac Zero-Copy Tx ...\n", 1, 2, 3, 4, 5, 6);

    pTxDma = &pDrvCtrl->txDma;

    pCurrentMblk = pMblk;
    while (pCurrentMblk != (M_BLK *)NULL)
        {
        /* point to next buffer descriptor */

        pTxDscr = &pTxDma->pDscrTable[pTxDma->tailIndex];

        pBuf = pCurrentMblk->mBlkHdr.mData;
        len = pCurrentMblk->mBlkHdr.mLen;
        offset = (UINT32)(pCurrentMblk->mBlkHdr.mData) & (CACHELINESIZE - 1);

        /* set buffer ptr, size in cache line */

        pTxDscr->dscr_a = (UINT64)((UINT32)KVTOPHYS (pBuf)) |
                          V_DMA_DSCRA_A_SIZE (NUMCACHEBLKS (offset + len));
        pTxDscr->dscr_b = V_DMA_DSCRB_OPTIONS (K_DMA_ETHTX_APPENDCRC_APPENDPAD);


        if (pCurrentMblk == pMblk)
            {
            /* set pkt len and  option */

            pTxDscr->dscr_a |= M_DMA_ETHTX_SOP;
            pTxDscr->dscr_b |= V_DMA_DSCRB_PKT_SIZE(
                                    pCurrentMblk->mBlkPktHdr.len);
            }

        DRV_LOG (DRV_DEBUG_TXD, "dscr_a = 0x%08x%08x\n",
                 (pTxDscr->dscr_a) >> 32, pTxDscr->dscr_a, 3, 4, 5, 6);
        DRV_LOG (DRV_DEBUG_TXD, "dscr_b = 0x%08x%08x\n",
                 (pTxDscr->dscr_b) >> 32, pTxDscr->dscr_b, 3, 4, 5, 6);

        /* Is this the last one? */

        if (pCurrentMblk->mBlkHdr.mNext == (M_BLK *)NULL)
            {
            pTxDscr->dscr_a |= M_DMA_DSCRA_INTERRUPT;

            /* save pointer to buffer and the buffer type. */

            pTxDma->bufTable[pTxDma->tailIndex].pBuf = (char *)pMblk;
            pTxDma->bufTable[pTxDma->tailIndex].type = BUF_TYPE_MBLK;
            }
        else
            {
            /*
             * Save pointer to buffer and the buffer type for 
             * the last descriptor.
             */

            pTxDma->bufTable[pTxDma->tailIndex].pBuf = (char *)NULL;
            pTxDma->bufTable[pTxDma->tailIndex].type = BUF_TYPE_NONE;
            }

        /* advance ring management variables */

        pTxDma->tailIndex = (pTxDma->tailIndex + 1) %  pTxDma->maxDescr;
        pTxDma->ringCount++;

        pCurrentMblk = pCurrentMblk->mBlkHdr.mNext;
        }

    /* tell hw one more dscr to go */

    ETH_DMA_REG_WRITE (pTxDma->regDscrCnt, fragNum);

    DRV_LOG (DRV_DEBUG_TX, "bcm1250Mac Zero-Copy Tx Done.\n",
             1, 2, 3, 4, 5, 6);
    return (OK);
    }

/*******************************************************************************
*
* bcm1250MacInt - entry point for handling interrupts
*
* The interrupting events are acknowledged to the device, so that the device
* will de-assert its interrupt signal.  The amount of work done here is kept
* to a minimum; the bulk of the work is deferred to the netTask.
*
* RETURNS: N/A
*/

LOCAL void bcm1250MacInt
    (
    DRV_CTRL *  pDrvCtrl    /* driver control structure */
    )
    {
    UINT64  status; /* Interrupt status */

    /* Get the interrupt status register value. */

    while ((status = ETH_MAC_REG_READ (pDrvCtrl->regIsr)) != 0)
        {
        DRV_LOG (DRV_DEBUG_INT, "status = 0x%x\n", status, 2, 3, 4, 5, 6);

        /* Check for receive channel 0 interrupt */

        if ((!pDrvCtrl->rxDma.hndlAct) &&
            (status & (M_MAC_INT_CHANNEL << S_MAC_RX_CH0)))
            {
            pDrvCtrl->rxDma.hndlAct = TRUE;
            (void)netJobAdd ((FUNCPTR)bcm1250MacRxHandle, (int)pDrvCtrl, 0,
                              0, 0, 0);
            }

        /* Check for transmit channel 0 interrupt */

        if ((!pDrvCtrl->txDma.hndlAct) &&
            (status & (M_MAC_INT_CHANNEL << S_MAC_TX_CH0)))
            {
            pDrvCtrl->txDma.hndlAct = TRUE;
            (void)netJobAdd ((FUNCPTR)bcm1250MacTxHandle, (int)pDrvCtrl, 0,
                              0, 0, 0);
            }
        }
    }


/*******************************************************************************
*
* bcm1250MacRxHandle - task-level routine to service receive frame interrupts
*
* This routine processes received frame interrupts, and runs at task level
* context in the netTask task.  Ethernet frames are received at the headIndex,
* and ethernet DMA hardware is given DMA buffer descriptors to receive into
* with the tailIndex.
*
* RETURNS: N/A
*/

LOCAL void bcm1250MacRxHandle
    (
    DRV_CTRL *  pDrvCtrl    /* driver control structure */
    )
    {
    ETH_MAC_DMA *   pRxDma;     /* ethernet DMA structure */
    ETH_DMA_DSCR *  pRxDscr;    /* DMA buffer descriptor */
    ETH_DMA_DSCR *  pCurrDscr;  /* points to current buffer descriptor */
    M_BLK_ID        pMblk;      /* mBlk pointer */
    CL_BLK_ID       pClBlk;     /* cBlk pointer */
    char *          pBuf;       /* cluster pointer */
    char *          pData;      /* received frame data pointer */
    int             len;        /* frame length */

    DRV_LOG (DRV_DEBUG_RX, "bcm1250MacRxHandle ......\n", 1, 2, 3, 4, 5, 6);

    pRxDma = &pDrvCtrl->rxDma;

    while (TRUE)
        {
        /* Point to next descriptor to be filled by the receiver. */

        pCurrDscr = (ETH_DMA_DSCR *)((UINT32)(ETH_DMA_REG_READ(
                                                 pRxDma->regCurDscr) &
                                              M_DMA_CURDSCR_ADDR));

        /* Point to next descriptor to be processed by this routine. */

        pRxDscr = &pRxDma->pDscrTable[pRxDma->headIndex];

        /* 
         * If all the full descriptors been processed, 
         * then break out of the loop and exit. 
         */

        if ((UINT32)pCurrDscr == KVTOPHYS (pRxDscr))
            {
            DRV_LOG (DRV_DEBUG_RX, "pRxDscr:0x%x catch pCurrPtr:0x%x\n",
                     (int)pRxDscr, (int)pCurrDscr, 3, 4, 5, 6);
            break;
            }

        /* Get pointer to received frame */

        pData = (char *)PHYSTOV ((UINT32)(pRxDscr->dscr_a));

        /* Check this received frame for an error. */

        if (pRxDscr->dscr_a & M_DMA_ETHRX_BAD)
            {
            DRV_LOG (DRV_DEBUG_RX, "bad frame\n", 1, 2, 3, 4, 5, 6);
            END_ERR_ADD (&pDrvCtrl->endObj, MIB2_IN_ERRS, 1);

            /* Point to next descriptor to be given to the hardware */

            pRxDscr = &pRxDma->pDscrTable[pRxDma->tailIndex];

            pRxDscr->dscr_a = KVTOPHYS ((UINT32)pData) |
                              V_DMA_DSCRA_A_SIZE (MAX_FRAME_CACHE_BLKS) |
                              M_DMA_DSCRA_INTERRUPT;
            pRxDscr->dscr_b = 0;

            /* mark the descriptor ready to receive */

            ETH_DMA_REG_WRITE (pRxDma->regDscrCnt, 1);

            /* advance ring management variables */

            pRxDma->tailIndex = (pRxDma->tailIndex + 1) % pRxDma->maxDescr;
            pRxDma->headIndex = (pRxDma->headIndex + 1) % pRxDma->maxDescr;
            continue;   /* back to top of while loop */
            }

        /* Process good frame received. */

        DRV_LOG (DRV_DEBUG_RX, "good frame\n", 1, 2, 3, 4, 5, 6);

        /* Update MIB-II variables */

        END_ERR_ADD (&pDrvCtrl->endObj, MIB2_IN_UCAST, 1);

        len = (int)G_DMA_DSCRB_PKT_SIZE (pRxDscr->dscr_b) - 4;

        pBuf = pData;
        DRV_LOG (DRV_DEBUG_RXD, "rx - pData= 0x%08x, len=%d\n", (int)pBuf,
                 len, 3, 4, 5, 6);
        DRV_LOG (DRV_DEBUG_RXD,
                 "rx - *pBuf= 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
                 pBuf[0], pBuf[1], pBuf[2], pBuf[3], pBuf[4], pBuf[5]);
        DRV_LOG (DRV_DEBUG_RXD,
                 "rx - *pBuf= 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
                 pBuf[6], pBuf[7], pBuf[8], pBuf[9], pBuf[10], pBuf[11]);
        DRV_LOG (DRV_DEBUG_RXD,
                 "rx - *pBuf= 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
                 pBuf[12], pBuf[13], pBuf[14], pBuf[15], pBuf[16], pBuf[17]);

        /* Allocate mBlk/clBlk/Cluster */

        pMblk = NET_MBLK_ALLOC ();
        pClBlk = NET_CL_BLK_ALLOC ();
        pBuf = NET_BUF_ALLOC ();

        /* If an allocation failed, discard the frame. */

        if ((pMblk == (M_BLK *)NULL) ||
            (pBuf == (char *)NULL) ||
            (pClBlk == (CL_BLK *)NULL))
            {
            DRV_LOG (DRV_DEBUG_RX, "not available RxBufs\n",
                     1, 2, 3, 4, 5, 6);
            END_ERR_ADD (&pDrvCtrl->endObj, MIB2_IN_ERRS, 1);
            pDrvCtrl->lastError.errCode = END_ERR_NO_BUF;
            muxError(&pDrvCtrl->endObj, &pDrvCtrl->lastError);

            /* Free any allocations that succeeded. */

            if (pMblk != (M_BLK *)NULL)
                NET_MBLK_FREE (pMblk);
            if (pBuf != (char *)NULL)
                NET_BUF_FREE (pBuf);
            if (pClBlk != (CL_BLK *)NULL)
                NET_CL_BLK_FREE (pClBlk);

            /* Point to next descriptor to be given to the hardware */

            pRxDscr = &pRxDma->pDscrTable[pRxDma->tailIndex];

            /* Re-arm this descriptor with the same buffer. */

            pRxDscr->dscr_a = KVTOPHYS ((UINT32)pData) |
                              V_DMA_DSCRA_A_SIZE (MAX_FRAME_CACHE_BLKS) |
                              M_DMA_DSCRA_INTERRUPT;
            pRxDscr->dscr_b = 0;

            /* mark the descriptor ready to receive */

            ETH_DMA_REG_WRITE (pRxDma->regDscrCnt, 1);

            /* advance ring management variables */

            pRxDma->tailIndex = (pRxDma->tailIndex + 1) % pRxDma->maxDescr;
            pRxDma->headIndex = (pRxDma->headIndex + 1) % pRxDma->maxDescr;
            continue;   /* back to top of while loop */
            }

        /* Point to next descriptor to be given to the hardware */

        pRxDscr = &pRxDma->pDscrTable[pRxDma->tailIndex];

        /* Re-arm this descriptor with the new buffer. */

        pRxDscr->dscr_a = KVTOPHYS ((UINT32)pBuf) |
                          V_DMA_DSCRA_A_SIZE (MAX_FRAME_CACHE_BLKS) |
                          M_DMA_DSCRA_INTERRUPT | VXW_RCV_BUF_OFFSET;
        pRxDscr->dscr_b = 0;

        /* mark the descriptor ready to receive */

        ETH_DMA_REG_WRITE (pRxDma->regDscrCnt, 1);

        /* send the frame to the upper layer */

        NET_CL_BLK_JOIN (pClBlk, pData - VXW_RCV_BUF_OFFSET,
                         MAX_FRAME_SIZE);
        NET_MBLK_CL_JOIN (pMblk, pClBlk);

        pMblk->mBlkHdr.mFlags |= M_PKTHDR;
        pMblk->mBlkHdr.mData = pData;
        pMblk->mBlkHdr.mLen = len;
        pMblk->mBlkPktHdr.len = pMblk->mBlkHdr.mLen;

        END_RCV_RTN_CALL (&pDrvCtrl->endObj, pMblk);

        DRV_LOG (DRV_DEBUG_RXD,
                 "rx - pMblk=0x%08x pClBlk=0x%08x pBuf=0x%08x\n",
                 (int)pMblk, (int)pClBlk, (int)pBuf, 4, 5, 6);

        /* advance ring management variables */

        pRxDma->tailIndex = (pRxDma->tailIndex + 1) % pRxDma->maxDescr;
        pRxDma->headIndex = (pRxDma->headIndex + 1) % pRxDma->maxDescr;
        }

    DRV_LOG (DRV_DEBUG_RX, "bcm1250MacRxHandle Done.\n", 1, 2, 3, 4, 5, 6);
    pRxDma->hndlAct = FALSE;
    }


/*******************************************************************************
*
* bcm1250MacTxHandle - task-level routine to service transmit frame interrupts
*
* This routine is processes transmit frame interrupts, and runs at task level
* context in the netTask task.
*
* RETURNS: N/A
*/

LOCAL void bcm1250MacTxHandle
    (
    DRV_CTRL *  pDrvCtrl    /* driver control structure */
    )
    {
    ETH_MAC_DMA *   pTxDma;             /* ethernet DMA structure */
    ETH_DMA_DSCR *  pTxDscr;            /* DMA buffer descriptor */
    ETH_DMA_DSCR *  pCurrDscr;          /* points to current descriptor */
    TX_BUF_TABLE *  pBufTbl;            /* points to current buffer table */
    BOOL            restart = FALSE;    /* mux restart flag */

    DRV_LOG (DRV_DEBUG_TX, "bcm1250MacTxHandle ......\n", 1, 2, 3, 4, 5, 6);

    pTxDma = &(pDrvCtrl->txDma);

    while (TRUE)
        {
        /* Point to next descriptor to be transmitted by the hardware. */

        pCurrDscr = (ETH_DMA_DSCR *)((UINT32)(ETH_DMA_REG_READ (
                                                 pTxDma->regCurDscr) &
                                              M_DMA_CURDSCR_ADDR));

        /* Point to next descriptor to be processed by this routine. */

        pTxDscr = &pTxDma->pDscrTable[pTxDma->headIndex];

        /* Have all the transmit complete descriptors been processed? */

        if ((UINT32)pCurrDscr == KVTOPHYS (pTxDscr))
             break;

        DRV_LOG (DRV_DEBUG_TX, "tx_remindex=0x%x, curdscr=0x%x, remdscr=0x%x\n",
                 pTxDma->headIndex, (int)pCurrDscr, (int)pTxDscr, 4, 5, 6);

        /* point to current descriptors buffer table entry. */

        pBufTbl = &pTxDma->bufTable[pTxDma->headIndex];

        /* free this descriptors buffer */

        if (pBufTbl->pBuf != (char *)NULL)
            {
            if (pBufTbl->type == BUF_TYPE_CL)
                {
                NET_BUF_FREE (pBufTbl->pBuf);
                }
            else if (pBufTbl->type == BUF_TYPE_MBLK)
                {
                netMblkClChainFree ((M_BLK *)pBufTbl->pBuf);
                }
            else
                {
                DRV_LOG (DRV_DEBUG_TX,
                         "unknown buffer type when try to release tx buf\n",
                         1, 2, 3, 4, 5, 6);
                }
            pBufTbl->pBuf = (char *)NULL;
            pBufTbl->type = BUF_TYPE_NONE;
            }

        /* Update descriptor index and count variables */

        END_TX_SEM_TAKE (&pDrvCtrl->endObj, WAIT_FOREVER);
        pTxDma->headIndex = (pTxDma->headIndex + 1) % pTxDma->maxDescr;
        pTxDma->ringCount--;
        END_TX_SEM_GIVE (&pDrvCtrl->endObj);

        /* Indicate that we need to check for the restart condition. */

        restart = TRUE;
        }

    /*
     * Does the Mux need to be restarted, and do we have plenty of DMA buffer
     * descriptors?
     */

    if ((restart) && (pTxDma->txBlocked) && (pTxDma->ringCount < 5))
        {
            pTxDma->txBlocked = FALSE;
            muxTxRestart ((void *)&pDrvCtrl->endObj);
        }

    DRV_LOG (DRV_DEBUG_TX, "bcm1250MacTxHandle Done.\n", 1, 2, 3, 4, 5, 6);

    /* Indicate that all transmit descriptors have been processed. */

    pTxDma->hndlAct = FALSE;
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
    DRV_CTRL *  pDrvCtrl,   /* driver control structure */
    M_BLK *     pMblk       /* mBlk to transmit */
    )
    {
    ETH_MAC_DMA *   pTxDma;     /* ethernet DMA structure */
    ETH_DMA_DSCR *  pTxDscr;    /* DMA buffer descriptor */
    ETH_DMA_DSCR *  pCurrDscr;  /* current descriptor */
    TX_BUF_TABLE *  pBufTbl;            /* points to current buffer table */
    int             len;        /* length of frame */

    DRV_LOG (DRV_DEBUG_TX, "bcm1250MacPollSend enter ......\n",
              1, 2, 3, 4, 5, 6);

    pTxDma = &pDrvCtrl->txDma;

    /* DMA buffer descriptors available? */

    if (pTxDma->ringCount >= pTxDma->maxDescr)
        {
        DRV_LOG (DRV_DEBUG_TX, "No available TxBufs\n", 1, 2, 3, 4, 5, 6);
        END_ERR_ADD (&pDrvCtrl->endObj, MIB2_OUT_ERRS, 1);

        return (EAGAIN); /* just return without freeing mBlk chain */
        }

    len = netMblkToBufCopy (pMblk, pTxPollBuf, (FUNCPTR)NULL);
    len = max (ETHERSMALL, len);

    DRV_LOG (DRV_DEBUG_TXD,
             "tx - *pTxPollBuf= 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
             pTxPollBuf[0], pTxPollBuf[1], pTxPollBuf[2], pTxPollBuf[3], pTxPollBuf[4], pTxPollBuf[5]);
    DRV_LOG (DRV_DEBUG_TXD,
             "tx - *pTxPollBuf= 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
             pTxPollBuf[6], pTxPollBuf[7], pTxPollBuf[8], pTxPollBuf[9], pTxPollBuf[10], pTxPollBuf[11]);
    DRV_LOG (DRV_DEBUG_TXD,
             "tx - *pTxPollBuf= 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
             pTxPollBuf[12], pTxPollBuf[13], pTxPollBuf[14], pTxPollBuf[15], pTxPollBuf[16], pTxPollBuf[17]);

    pTxDscr = &pTxDma->pDscrTable[pTxDma->tailIndex];

    /* set txbuf ptr, cache line size, ...  */

    pTxDscr->dscr_a = (UINT64)((UINT32)KVTOPHYS (pTxPollBuf)) |
                      V_DMA_DSCRA_A_SIZE (NUMCACHEBLKS (len)) |
                      M_DMA_ETHTX_SOP;

    /* set pkt len and option */

    pTxDscr->dscr_b = V_DMA_DSCRB_OPTIONS (K_DMA_ETHTX_APPENDCRC_APPENDPAD) |
                      V_DMA_DSCRB_PKT_SIZE (len);

    /* save the buf info */

    pTxDma->bufTable[pTxDma->tailIndex].pBuf = pTxPollBuf;
    pTxDma->bufTable[pTxDma->tailIndex].type = BUF_TYPE_CL;

    /* Advance ring management variables */

    pTxDma->tailIndex = (pTxDma->tailIndex + 1) % pTxDma->maxDescr;
    pTxDma->ringCount++;

    ETH_DMA_REG_WRITE (pTxDma->regDscrCnt, 1);

    while (TRUE)
        {
        /* Point to next descriptor to be transmitted by the hardware. */

        pCurrDscr = (ETH_DMA_DSCR *)((UINT32)(ETH_DMA_REG_READ (
                                                 pTxDma->regCurDscr) &
                                              M_DMA_CURDSCR_ADDR));

        /* Point to next descriptor to be processed by this routine. */
        
        pTxDscr = &pTxDma->pDscrTable[pTxDma->headIndex];

        /* Have all the transmit complete descriptors been processed? */

        if ((UINT32)pCurrDscr == KVTOPHYS (pTxDscr))
            break;

        DRV_LOG (DRV_DEBUG_TX, 
                 "TX headIndex:0x%x, pCurrDscr=0x%x, pTxDscr=0x%x\n",
                 pTxDma->headIndex, (int)pCurrDscr, (int)pTxDscr, 4, 5, 6);

        /* point to current descriptors buffer table entry. */

        pBufTbl = &pTxDma->bufTable[pTxDma->headIndex];

        /* free this descriptors buffer */
 
        if (pBufTbl->pBuf != (char *)NULL)
            {
            pBufTbl->pBuf = (char *)NULL;
            pBufTbl->type = BUF_TYPE_NONE;
            }

        /* Update ring management variables */

        pTxDma->headIndex = (pTxDma->headIndex + 1) % pTxDma->maxDescr;
        pTxDma->ringCount--;
        }

    DRV_LOG (DRV_DEBUG_TX, "bcm1250MacPollSend Done\n", 1, 2, 3, 4, 5, 6);

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
    DRV_CTRL *  pDrvCtrl,   /* driver control structure */
    M_BLK *     pMblk       /* mBlk to receive into */
    )
    {
    ETH_MAC_DMA *   pRxDma;     /* ethernet DMA structure */
    ETH_DMA_DSCR *  pRxDscr;    /* DMA buffer descriptor */
    ETH_DMA_DSCR *  pCurrDscr;  /* current descriptor */
    char *          pData;      /* frame data pointer */
    int             len;        /* frame data length */

    DRV_LOG (DRV_DEBUG_RX, "bcm1250MacPollRcv enter ......\n",
             1, 2, 3, 4, 5, 6);

    pRxDma = &pDrvCtrl->rxDma;

    /* Point to next descriptor to be filled by the receiver. */

    pCurrDscr = (ETH_DMA_DSCR *)((UINT32)(ETH_DMA_REG_READ (
                                             pRxDma->regCurDscr) &
                                          M_DMA_CURDSCR_ADDR));

    /* Point to next descriptor to be processed by this routine. */

    pRxDscr = &pRxDma->pDscrTable[pRxDma->headIndex];

    DRV_LOG (DRV_DEBUG_RX, "RX headIndex=%d, pRxDscr=0x%x pCurrDscr=0x%x\n",
                            pRxDma->headIndex, (int)pRxDscr,
                            (int)pCurrDscr, 4, 5, 6);

    /* 
     * If all the full descriptors been processed, 
     * then break out of the loop. 
     */

    if ((UINT32)pCurrDscr == KVTOPHYS (pRxDscr))
        {
        DRV_LOG (DRV_DEBUG_RX, "bcm1250MacPollRcv done\n", 1, 2, 3, 4, 5, 6);
        return (EAGAIN);
        }

    DRV_LOG (DRV_DEBUG_RX, "RX headIndex=%d, pRxDscr=0x%x pCurrDscr=0x%x\n",
                            pRxDma->headIndex, (int)pRxDscr,
                            (int)pCurrDscr, 4, 5, 6);

    /* Check this received frame for an error. */

    if (pRxDscr->dscr_a & M_DMA_ETHRX_BAD)
        {
        DRV_LOG (DRV_DEBUG_RX, "Bad Frame\n", 1, 2, 3, 4, 5, 6);
        END_ERR_ADD (&pDrvCtrl->endObj, MIB2_IN_ERRS, 1);

        pData = (char *)PHYSTOV ((UINT32)(pRxDscr->dscr_a));

        pRxDscr = &pRxDma->pDscrTable[pRxDma->tailIndex];
        pRxDscr->dscr_a = KVTOPHYS ((UINT32)pData) |
                          V_DMA_DSCRA_A_SIZE (MAX_FRAME_CACHE_BLKS);
        pRxDscr->dscr_b = 0;

        /* mark the descriptor ready to receive  again */

        ETH_DMA_REG_WRITE (pRxDma->regDscrCnt, 1);

        /* advance ring management variables */

        pRxDma->tailIndex = (pRxDma->tailIndex + 1) % pRxDma->maxDescr;
        pRxDma->headIndex = (pRxDma->headIndex + 1) % pRxDma->maxDescr;
        DRV_LOG (DRV_DEBUG_RX, "bcm1250MacPollRcv done\n", 1, 2, 3, 4, 5, 6);
        return (EAGAIN);
        }

    DRV_LOG (DRV_DEBUG_RX, "Good Frame\n", 1, 2, 3, 4, 5, 6);
    END_ERR_ADD (&pDrvCtrl->endObj, MIB2_IN_UCAST, 1);

    len = (int)G_DMA_DSCRB_PKT_SIZE (pRxDscr->dscr_b) - ETH_CRC_LEN;
    pData = (char *)PHYSTOV ((UINT32)(pRxDscr->dscr_a));

    DRV_LOG (DRV_DEBUG_RXD, "rx - pData= 0x%08x, len=%d\n",
             (int)pData, len, 3, 4, 5, 6);
    DRV_LOG (DRV_DEBUG_RXD,
             "rx - *pData= 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
             pData[0], pData[1], pData[2], pData[3], pData[4], pData[5]);
    DRV_LOG (DRV_DEBUG_RXD,
             "rx - *pData= 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
             pData[6], pData[7], pData[8], pData[9], pData[10], pData[11]);
    DRV_LOG (DRV_DEBUG_RXD,
             "rx - *pData= 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
             pData[12], pData[13], pData[14], pData[15], pData[16],
             pData[17]);

    pMblk->mBlkHdr.mFlags |= M_PKTHDR;
    pMblk->mBlkHdr.mLen = len;
    pMblk->mBlkPktHdr.len = pMblk->mBlkHdr.mLen;
    pMblk->mBlkHdr.mData += VXW_RCV_BUF_OFFSET;

    bcopy (pData, (char *)pMblk->mBlkHdr.mData, len);

    pRxDscr = &pRxDma->pDscrTable[pRxDma->tailIndex];

    pRxDscr->dscr_a = KVTOPHYS ((UINT32)pData) |
                      V_DMA_DSCRA_A_SIZE (MAX_FRAME_CACHE_BLKS) | 2;
    pRxDscr->dscr_b = 0;

    /* mark the descriptor ready to receive again */

    ETH_DMA_REG_WRITE (pRxDma->regDscrCnt, 1);

    /* advance ring management variables */

    pRxDma->tailIndex = (pRxDma->tailIndex + 1) % pRxDma->maxDescr;
    pRxDma->headIndex = (pRxDma->headIndex + 1) % pRxDma->maxDescr;

    return OK;
    }


/*******************************************************************************
*
* bsm1250MacPollStart - start polled mode operations
*
* RETURNS: N/A
*/

LOCAL void bcm1250MacPollStart
    (
    DRV_CTRL *  pDrvCtrl    /* driver control structure */
    )
    {
    int         oldLevel;   /* saved interrupt level */

    DRV_LOG (DRV_DEBUG_POLL, "PollStart enter ......\n", 1, 2, 3, 4, 5, 6);

    oldLevel = intLock ();

    DRV_FLAGS_SET (BCM1250_MAC_POLLING);

    /* diasble VxW interrupt */

    (void)bcm1250IntDisable (pDrvCtrl->intSource);

    intUnlock (oldLevel);

    DRV_LOG (DRV_DEBUG_POLL, "PollStart done\n", 1, 2, 3, 4, 5, 6);
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
* RETURNS: N/A
*/

LOCAL void bcm1250MacPollStop
    (
    DRV_CTRL *  pDrvCtrl    /* driver control structure */
    )
    {
    int         oldLevel;   /* saved interrupt level */

    DRV_LOG (DRV_DEBUG_POLL, "PollStop enter ......\n", 1, 2, 3, 4, 5, 6);

    oldLevel = intLock ();

    (void)bcm1250IntEnable (pDrvCtrl->intSource);

    DRV_FLAGS_CLR (BCM1250_MAC_POLLING);

    intUnlock (oldLevel);

    DRV_LOG (DRV_DEBUG_POLL, "PollStop done\n", 1, 2, 3, 4, 5, 6);
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
    DRV_CTRL *  pDrvCtrl,   /* driver control structure */
    int         cmd,        /* command to be executed */
    caddr_t     data        /* data associated with command */
    )
    {
    int         error = OK; /* command error flag */
    long        endFlags;   /* END_OBJ flags */
    int         drvFlags;   /* driver flags */
    UINT64      reg;        /* value to read/write to register */
    END_OBJ *   pEndObj = &pDrvCtrl->endObj;

    DRV_LOG (DRV_DEBUG_IOCTL, "Ioctl unit=0x%x cmd=%d data=0x%x\n",
             pDrvCtrl->unit, cmd, (int)data, 4, 5, 6);

    switch (cmd)
        {
        case EIOCSADDR:
            DRV_LOG (DRV_DEBUG_IOCTL, "EIOCSADDR\n", 1, 2, 3, 4, 5, 6);
            if (data == (caddr_t)NULL)
                error = EINVAL;
            else
                {
                /* Copy and install the new address */
                bcopy ((char *)data, (char *)END_HADDR (pEndObj),
                       END_HADDR_LEN (pEndObj));

                reg =  bcm1250MacAddr2Reg ((unsigned char *)data);
                DRV_LOG (DRV_DEBUG_IOCTL, "mac addr = 0x%08x%08x\n",
                         (int)(reg >> 32), reg, 3, 4, 5, 6);

                /* need pass 1 bug workaround */
                ETH_MAC_REG_WRITE (R_MAC_ADDR_BASE, reg);

                ETH_MAC_REG_WRITE (R_MAC_ETHERNET_ADDR, reg);
                }
            break;

        case EIOCGADDR:
            DRV_LOG (DRV_DEBUG_IOCTL, "EIOCGADDR\n", 1, 2, 3, 4, 5, 6);
            if (data == (caddr_t)NULL)
                error = EINVAL;
            else
                bcopy ((char *)END_HADDR (pEndObj), (char *)data,
                       END_HADDR_LEN (pEndObj));
            break;

        case EIOCSFLAGS:
            DRV_LOG (DRV_DEBUG_IOCTL, "EIOCSAFLGS\n", 1, 2, 3, 4, 5, 6);
            endFlags = (long)data;
            if (endFlags < 0)
                {
                endFlags = -endFlags;
                endFlags--;
                END_FLAGS_CLR (pEndObj, endFlags);
                }
            else
                {
                END_FLAGS_SET (pEndObj, endFlags);
                }

            /* handle IFF_PROMISC */
            drvFlags = DRV_FLAGS_GET ();
            if (END_FLAGS_ISSET (pEndObj, IFF_PROMISC))
                DRV_FLAGS_SET (BCM1250_MAC_PROMISC);
            else
                DRV_FLAGS_CLR (BCM1250_MAC_PROMISC);

            if (END_FLAGS_GET (pEndObj) & (IFF_MULTICAST | IFF_ALLMULTI))
                DRV_FLAGS_SET (BCM1250_MAC_MCAST);
            else
                DRV_FLAGS_CLR (BCM1250_MAC_MCAST);

            DRV_LOG (DRV_DEBUG_IOCTL,
                     "endFlags=0x%x drvFlags=0x%x newFlags=0x%x\n",
                     END_FLAGS_GET (pEndObj), drvFlags, DRV_FLAGS_GET (),
                     4, 5, 6);

            if ((DRV_FLAGS_GET () != drvFlags) &&
                (END_FLAGS_GET (pEndObj) & IFF_UP))
                bcm1250MacRxFilterSet (pDrvCtrl);
            break;

        case EIOCGFLAGS:
            DRV_LOG (DRV_DEBUG_IOCTL, "EIOCGFLAGS: 0x%x: 0x%x\n",
                    pEndObj->flags, *(long *)data, 3, 4, 5, 6);

            if (data == (caddr_t)NULL)
                error = EINVAL;
            else
                *(long *)data = END_FLAGS_GET (pEndObj);
            break;

        case EIOCMULTIADD:
            DRV_LOG (DRV_DEBUG_IOCTL, "EIOCMULTIADD\n", 1, 2, 3, 4, 5, 6);
            error = bcm1250MacMCastAdd (pDrvCtrl, (char *)data);
            break;

        case EIOCMULTIDEL:
            DRV_LOG (DRV_DEBUG_IOCTL, "EIOCMULTIDEL\n", 1, 2, 3, 4, 5, 6);
            error = bcm1250MacMCastDel (pDrvCtrl, (char *)data);
            break;

        case EIOCMULTIGET:
            DRV_LOG (DRV_DEBUG_IOCTL, "EIOCMULITGET\n", 1, 2, 3, 4, 5, 6);
            error = bcm1250MacMCastGet (pDrvCtrl, (MULTI_TABLE *)data);
            break;

        case EIOCPOLLSTART:
            DRV_LOG (DRV_DEBUG_IOCTL, "EIOCPOLLSTART\n", 1, 2, 3, 4, 5, 6);
            bcm1250MacPollStart (pDrvCtrl);
            break;

        case EIOCPOLLSTOP:
            DRV_LOG (DRV_DEBUG_IOCTL, "EIOCPOLLSTOP\n", 1, 2, 3, 4, 5, 6);
            bcm1250MacPollStop (pDrvCtrl);
            break;

        case EIOCGMIB2:                      /* move to mux */
            DRV_LOG (DRV_DEBUG_IOCTL, "EIOCGMIBS2\n", 1, 2, 3, 4, 5, 6);
            if (data == (caddr_t)NULL)
                error = EINVAL;
            else
                bcopy ((char *)&pEndObj->mib2Tbl, (char *)data,
                       sizeof (pEndObj->mib2Tbl));
            break;

        default:
            DRV_LOG (DRV_DEBUG_IOCTL, "IOCTL Invalid Command\n",
                                        1, 2, 3, 4, 5, 6);
            error = EINVAL;
            break;
        }

    return (error);
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
    DRV_CTRL *  pDrvCtrl,   /* driver control structure */
    char *      pAddr       /* multicast address */
    )
    {
    int     rtv;    /* function return value */

    DRV_LOG (DRV_DEBUG_IOCTL, "bcm1250MacMCastAdd\n", 1, 2, 3, 4, 5, 6);

    rtv = etherMultiAdd (&pDrvCtrl->endObj.multiList, pAddr);

    if (rtv == ENETRESET)
        {
        pDrvCtrl->endObj.nMulti++;

        return (bcm1250MacHashRegAdd (pDrvCtrl, pAddr));
        }

    return ((rtv == OK) ? OK : ERROR);
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
    DRV_CTRL *  pDrvCtrl,   /* driver control structure */
    char *      pAddr       /* multicast address */
    )
    {
    int     rtv;    /* function return value */

    DRV_LOG (DRV_DEBUG_IOCTL, "bcm1250MacMCastDel\n", 1, 2, 3, 4, 5, 6);

    rtv = etherMultiDel (&pDrvCtrl->endObj.multiList, pAddr);

    if (rtv == ENETRESET)
        {
        pDrvCtrl->endObj.nMulti--;

        return (bcm1250MacHashRegSet (pDrvCtrl));
        }

    return ((rtv == OK) ? OK : ERROR);
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
    DRV_CTRL *      pDrvCtrl,   /* driver control structure */
    MULTI_TABLE *   pTable      /* multicast address table */
    )
    {
    DRV_LOG (DRV_DEBUG_IOCTL, "sbMCastGet\n", 1, 2, 3, 4, 5, 6);

    return (etherMultiGet (&pDrvCtrl->endObj.multiList, pTable));
    }


/*******************************************************************************
*
* bcm1250MacHashRegAdd - set Receiver Hash table registers
*
* This routine sets the Receiver Hash table registers.
*
* RETURNS: OK
*/

LOCAL STATUS bcm1250MacHashRegAdd
    (
    DRV_CTRL *  pDrvCtrl,   /* driver control structure */
    char     *  pAddr       /* multicast address */
    )
    {
    int     dwOffset;
    int     hashVal;
    UINT64  reg;
    UINT64  oldReg;

    DRV_LOG (DRV_DEBUG_IOCTL, "MacHashRegAdd enter ......\n",
             1, 2, 3, 4, 5, 6);

    hashVal = bcm1250MacEthHash ((unsigned char *)pAddr);

    dwOffset = hashVal >> 6;
    reg = 1 << ((hashVal & 0x3F) - 1);

    oldReg = ETH_MAC_REG_READ (R_MAC_HASH_BASE + (dwOffset * 8));
    ETH_MAC_REG_WRITE (R_MAC_HASH_BASE + (dwOffset * 8), (reg | oldReg));

    DRV_LOG (DRV_DEBUG_IOCTL, "MacHashRegAdd done\n", 1, 2, 3, 4, 5, 6);

    return (OK);
    }


/*******************************************************************************
*
* bcm1250MacHashRegSet - set Receiver Hash table registers
*
* This routine sets the Receiver Hash table registers.
*
* RETURNS: OK
*/

LOCAL STATUS bcm1250MacHashRegSet
    (
    DRV_CTRL *  pDrvCtrl       /* driver control structure */
    )
    {
    int             dwOffset;
    int             hashVal;
    int             idx;
    UINT64          reg;
    UINT64          oldReg;
    MAC_REG         port;
    ETHER_MULTI *   mCastNode = (ETHER_MULTI *)NULL;

    DRV_LOG (DRV_DEBUG_IOCTL, "MacHashRegSet enter ......\n",
             1, 2, 3, 4, 5, 6);

    /* clear hash table */

    port = R_MAC_HASH_BASE;
    for (idx = 0; idx < MAC_HASH_COUNT; idx++)
        {
        ETH_MAC_REG_WRITE (port, 0);
        port += sizeof (UINT64);
        }

    port = R_MAC_HASH_BASE;

    for (mCastNode = (ETHER_MULTI *)lstFirst (&pDrvCtrl->endObj.multiList);
             mCastNode != (ETHER_MULTI *)NULL;
                 mCastNode = (ETHER_MULTI *)lstNext (&mCastNode->node))
        {
        hashVal = bcm1250MacEthHash ((unsigned char *)mCastNode->addr);

        dwOffset = hashVal >> 6;
        reg = 1 << ((hashVal & 0x3F) - 1);

        oldReg = ETH_MAC_REG_READ (port + (dwOffset * 8));
        ETH_MAC_REG_WRITE (port + (dwOffset * 8), (reg | oldReg));
        }

    DRV_LOG (DRV_DEBUG_IOCTL, "MacHashRegSet done\n", 1, 2, 3, 4, 5, 6);

    return (OK);
    }


/*******************************************************************************
*
* bcm1250MacEthHash - calculate destination address hash value
*
* This routine calculates the destination address hash value.
* See the bcm1250 manual, pages 9-190.
*
* RETURNS: the destination address hash value.
*/

LOCAL unsigned bcm1250MacEthHash
    (
    unsigned char * pAddr   /* destination address */
    )
    {
    int             idx;
    unsigned int    crc = 0xFFFFFFFFUL;

    static unsigned int crcTab[] =
                        {
                        0x00000000, 0x1DB71064, 0x3B6E20C8, 0x26D930AC,
                        0x76DC4190, 0x6B6B51F4, 0x4DB26158, 0x5005713C,
                        0xEDB88320, 0xF00F9344, 0xD6D6A3E8, 0xCB61B38C,
                        0x9B64C2B0, 0x86D3D2D4, 0xA00AE278, 0xBDBDF21C
                        };

    for (idx = 0; idx < ETH_ADDR_LEN; idx++)
        {
        crc ^= *pAddr++;
        crc = (crc >> 4) ^ crcTab[crc & 0xF];
        crc = (crc >> 4) ^ crcTab[crc & 0xF];
        }
    return ((crc >> 1) & 0x1FF);
}


/*******************************************************************************
*
* bcm1250MacRxFilterSet - set the rx filter register
*
* This routine sets the rx filter register according to the DRV FLAGS.
*
* RETURNS: N/A
*/

LOCAL void bcm1250MacRxFilterSet
    (
    DRV_CTRL    *pDrvCtrl   /* driver control structure */
    )
    {
    UINT64  rxFilterCfg;    /* RX filter register value */

    DRV_LOG (DRV_DEBUG_IOCTL, "RxFilter\n", 1, 2, 3, 4, 5, 6);

    /* read the current rx filter reg */
    rxFilterCfg = ETH_MAC_REG_READ (pDrvCtrl->regRxFilter);

    if (DRV_FLAGS_ISSET (BCM1250_MAC_MCAST))
        rxFilterCfg |= M_MAC_MCAST_EN;

    if (DRV_FLAGS_ISSET (BCM1250_MAC_PROMISC))
        rxFilterCfg |= M_MAC_ALLPKT_EN;

    ETH_MAC_REG_WRITE (pDrvCtrl->regRxFilter, rxFilterCfg);
    }


/*******************************************************************************
*
* bcm1250MacSetConfig - configure MAC options
*
* The routine configures the LAN speed and the Ethernet duplex and flow control 
* options for this MAC.
*
* RETURNS: OK if successful or ERROR if the parameters are invalid
*/

LOCAL STATUS bcm1250MacSetConfig
    (
    DRV_CTRL *  pDrvCtrl,   /* driver control structure */
    MAC_SPEED   speed,      /* speed to set MAC to */
    MAC_DUPLEX  duplex,     /* duplex setting */
    MAC_FC      fc          /* flow control setting */
    )
    {
    UINT64  macCfg;     /* MAC config register value */
    UINT64  frameCfg;   /* frame config register value */

    /* Read current register values */

    macCfg = ETH_MAC_REG_READ (pDrvCtrl->regMacCfg);
    frameCfg = ETH_MAC_REG_READ (pDrvCtrl->regFrameCfg);

    /* Mask out the stuff we want to change */

    macCfg &= ~(M_MAC_BURST_EN | M_MAC_SPEED_SEL | M_MAC_FC_SEL | 
                M_MAC_FC_CMD   | M_MAC_HDX_EN);
    frameCfg &= ~(M_MAC_IFG_RX | M_MAC_IFG_TX | M_MAC_IFG_THRSH |
                  M_MAC_SLOT_SIZE);

     /* Now add in the new bits */

    switch (speed)
        {
        case MAC_SPEED_10:
            frameCfg |= V_MAC_IFG_RX_10 |
                        V_MAC_IFG_TX_10 |
                        K_MAC_IFG_THRSH_10 |
                        V_MAC_SLOT_SIZE_10;
            macCfg |= V_MAC_SPEED_SEL_10MBPS;
            break;

        case MAC_SPEED_100:
            frameCfg |= V_MAC_IFG_RX_100 |
                        V_MAC_IFG_TX_100 |
                        V_MAC_IFG_THRSH_100 |
                        V_MAC_SLOT_SIZE_100;
            macCfg |= V_MAC_SPEED_SEL_100MBPS ;
            break;

        case MAC_SPEED_1000:
            frameCfg |= V_MAC_IFG_RX_1000 |
                        V_MAC_IFG_TX_1000 |
                        V_MAC_IFG_THRSH_1000 |
                        V_MAC_SLOT_SIZE_1000;
            macCfg |= V_MAC_SPEED_SEL_1000MBPS | M_MAC_BURST_EN;
            break;

        default:
            return ERROR;
        }

    /* Send the bits back to the hardware */

    ETH_MAC_REG_WRITE (pDrvCtrl->regFrameCfg, frameCfg);

    switch (duplex)
        {
        case MAC_DUPLEX_HALF:
            switch (fc)
                {
                case MAC_FC_DISABLED:
                    macCfg |= M_MAC_HDX_EN | V_MAC_FC_CMD_DISABLED;
                    break;

                case MAC_FC_COLLISION:
                    macCfg |= M_MAC_HDX_EN | V_MAC_FC_CMD_ENABLED;
                    break;

                case MAC_FC_CARRIER:
                    macCfg |= M_MAC_HDX_EN | V_MAC_FC_CMD_ENAB_FALSECARR;
                    break;

                case MAC_FC_FRAME:            /* not valid in half duplex */
                default:                        /* invalid selection */
                    return ERROR;
                }
            macCfg &= ~(M_MAC_BURST_EN);
            break;

        case MAC_DUPLEX_FULL:
            switch (fc)
                {
                case MAC_FC_DISABLED:
                    macCfg |= V_MAC_FC_CMD_DISABLED;
                    break;

                case MAC_FC_FRAME:
                    macCfg |= V_MAC_FC_CMD_ENABLED;
                    break;

                case MAC_FC_COLLISION:        /* not valid in full duplex */
                case MAC_FC_CARRIER:          /* not valid in full duplex */
                    /* fall through */
                default:
                    return ERROR;
                }
            break;
        }

    /* Send the bits back to the hardware */

    ETH_MAC_REG_WRITE (pDrvCtrl->regMacCfg, macCfg);

    return OK;
    }


/*******************************************************************************
*
* bcm1250MacAddr2Reg - convert a 6-byte MAC address to a 64-bit value
*
* This routine converts the hardware ethernet MAC address from an array of
* six-byte values to a 64-bit register value.
*
* RETURNS: the 64-bit hardware ethernet MAC address
*/

LOCAL UINT64 bcm1250MacAddr2Reg
    (
    unsigned char * ptr /* points to 6 byte MAC address */
    )
    {
    UINT64 reg = 0; /* 64 bit ethernet MAC address */

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


/*******************************************************************************
*
* bcm1250MacEthMiiSync - send sync bits to physical interface chip
*
* RETURNS: N/A
*/

LOCAL void bcm1250MacEthMiiSync
    (
    DRV_CTRL *  pDrvCtrl    /* driver control structure */
    )
    {
    int     cnt;    /* loop count */
    UINT64  bits;   /* holds sync bits */

    bits = M_MAC_MDIO_DIR_OUTPUT | M_MAC_MDIO_OUT;

    ETH_MAC_REG_WRITE (pDrvCtrl->regMdio, bits);

    for (cnt = 0; cnt < 32; cnt++)
        {
        ETH_MAC_REG_WRITE (pDrvCtrl->regMdio, bits | M_MAC_MDC);
        ETH_MAC_REG_WRITE (pDrvCtrl->regMdio, bits);
        }
    }


/*******************************************************************************
*
* bcm1250MacEthMiiSendData - send number of bits to physical interface chip
*
* RETURNS: N/A
*/

LOCAL void bcm1250MacEthMiiSendData
    (
    DRV_CTRL *      pDrvCtrl,   /* driver control structure */
    unsigned int    data,       /* data bits to send */
    int             bitCnt      /* number of bits to send */
    )
    {
    int             i;          /* loop count */
    UINT64          bits;       /* bits to send */
    unsigned int    curMask;    /* bit mask */

    bits = M_MAC_MDIO_DIR_OUTPUT;
    ETH_MAC_REG_WRITE (pDrvCtrl->regMdio, bits);

    curMask = 1 << (bitCnt - 1);

    for (i = 0; i < bitCnt; i++)
        {
        if (data & curMask)
            bits |= M_MAC_MDIO_OUT;
        else
            bits &= ~M_MAC_MDIO_OUT;
        ETH_MAC_REG_WRITE (pDrvCtrl->regMdio, bits);
        ETH_MAC_REG_WRITE (pDrvCtrl->regMdio, bits | M_MAC_MDC);
        ETH_MAC_REG_WRITE (pDrvCtrl->regMdio, bits);
        curMask >>= 1;
        }
    }


/*******************************************************************************
*
* bcm1250MacEthMiiRead - read from physical interface chip
*
* First send sync bits, then START and READ command, followed by physical
* interface chip address and register index.  Next switch to input mode and
* clock in physical interface chip data.
*
* RETURNS: Value read from physical interface chip.
*/

LOCAL int bcm1250MacEthMiiRead
    (
    DRV_CTRL *  pDrvCtrl,   /* driver control structure */
    int         phyAddr,    /* physical interface chip address */
    int         regIdx      /* physical interface chip register index */
    )
    {
    int     idx;    /* loop index */
    int     error;  /* error flag */
    int     regVal; /* register value */

    /*
     * Synchronize ourselves so that the PHY knows the next
     * thing coming down is a command
     */

    bcm1250MacEthMiiSync (pDrvCtrl);

    /*
     * Send the data to the PHY.  The sequence is
     * a "start" command (2 bits)
     * a "read" command (2 bits)
     * the PHY addr (5 bits)
     * the register index (5 bits)
     */

    bcm1250MacEthMiiSendData (pDrvCtrl, MII_COMMAND_START, 2);
    bcm1250MacEthMiiSendData (pDrvCtrl, MII_COMMAND_READ, 2);
    bcm1250MacEthMiiSendData (pDrvCtrl, phyAddr, 5);
    bcm1250MacEthMiiSendData (pDrvCtrl, regIdx, 5);

    /*
     * Switch the port around without a clock transition.
     */

    ETH_MAC_REG_WRITE (pDrvCtrl->regMdio, M_MAC_MDIO_DIR_INPUT);

    /*
     * Send out a clock pulse to signal we want the status
     */

    ETH_MAC_REG_WRITE (pDrvCtrl->regMdio, M_MAC_MDIO_DIR_INPUT | M_MAC_MDC);
    ETH_MAC_REG_WRITE (pDrvCtrl->regMdio, M_MAC_MDIO_DIR_INPUT);

    /*
     * If an error occured, the PHY will signal '1' back
     */

    error = ETH_MAC_REG_READ (pDrvCtrl->regMdio) & M_MAC_MDIO_IN;

    /*
     * Issue an 'idle' clock pulse, but keep the direction
     * the same.
     */

    ETH_MAC_REG_WRITE (pDrvCtrl->regMdio, M_MAC_MDIO_DIR_INPUT | M_MAC_MDC);
    ETH_MAC_REG_WRITE (pDrvCtrl->regMdio, M_MAC_MDIO_DIR_INPUT);

    regVal = 0;
    for (idx = 0; idx < 16; idx++)
        {
        regVal <<= 1;

        if (error == 0)
            {
            if (ETH_MAC_REG_READ (pDrvCtrl->regMdio) & M_MAC_MDIO_IN)
                regVal |= 1;
            }

        ETH_MAC_REG_WRITE (pDrvCtrl->regMdio,
                           M_MAC_MDIO_DIR_INPUT | M_MAC_MDC);
        ETH_MAC_REG_WRITE (pDrvCtrl->regMdio, M_MAC_MDIO_DIR_INPUT);
        }

    if (error != 0)
        regVal = 0;

    /* Switch back to output */

    ETH_MAC_REG_WRITE (pDrvCtrl->regMdio, M_MAC_MDIO_DIR_OUTPUT);

    return regVal;
    }


/*******************************************************************************
*
* bcm1250MacEthMiiWrite - write to physical register
*
* This routine first sends START and WRITE commands, followed by the address of 
* the physical interface chip, and then register index. It then sends ACK and 
* the register value to be written.
*
* RETURNS: N/A
*/

#ifdef FUTURE_USE
LOCAL void bcm1250MacEthMiiWrite
    (
    DRV_CTRL *      pDrvCtrl,   /* driver control structure */
    int             phyAddr,    /* physical interface chip address */
    int             regIdx,     /* register in physical interface chip */
    unsigned int    regVal      /* value to write to regIdx */
    )
    {

    bcm1250MacEthMiiSync (pDrvCtrl);

    bcm1250MacEthMiiSendData (pDrvCtrl, MII_COMMAND_START, 2);
    bcm1250MacEthMiiSendData (pDrvCtrl, MII_COMMAND_WRITE, 2);
    bcm1250MacEthMiiSendData (pDrvCtrl, phyAddr, 5);
    bcm1250MacEthMiiSendData (pDrvCtrl, regIdx, 5);
    bcm1250MacEthMiiSendData (pDrvCtrl, MII_COMMAND_ACK, 2);
    bcm1250MacEthMiiSendData (pDrvCtrl, regVal, 16);

    ETH_MAC_REG_WRITE (pDrvCtrl->regMdio, M_MAC_MDIO_DIR_OUTPUT);
    }
#endif /* FUTURE_USE */


/*******************************************************************************
*
* bcm1250MacEthMiiFindPhy - search for a physical interface chip
*
* This routine reads from all the possible physical interface chip addresses
* checking for a valid interface chip.  If one is found, its address is saved
* in the driver control structure.
*
* RETURNS: N/A
*/

LOCAL void bcm1250MacEthMiiFindPhy
    (
    DRV_CTRL *  pDrvCtrl    /* driver control structure */
    )
    {
    int     phy;    /* physical interface chip address */
    int     bmsr;   /* value of Basic Mode Status Register */

    for (phy = 0; phy < 31; phy++)
        {
        bmsr = bcm1250MacEthMiiRead (pDrvCtrl, phy, MII_BMSR);
        if (bmsr != 0)
            {
            pDrvCtrl->phyAddr = phy;
            return;
            }
        }
    (void)printf ("Found no PHY\n");

    pDrvCtrl->phyAddr = 1;
    }


/*******************************************************************************
*
* bcm1250MacEthMiiPoll - poll and configure speed
*
* Ask the PHY what is going on, and configure speed appropriately.
* For the moment, we only support automatic configuration.
*
* Input parameters:
*     s - sbeth structure
*
* RETURNS: N/A
*/

LOCAL void bcm1250MacEthMiiPoll
    (
    DRV_CTRL *  pDrvCtrl    /* driver control structure */
    )
    {
    int     bmsr;   /* value of Basic Mode Status Register */
    int     bmcr;   /* value of Basic Mode Control Register */
    int     k1stsr; /* value of 1K Status Register */
    int     anlpar; /* value of Autonegotiation lnk partner abilities */

    /* Read the mode status and mode control registers. */

    bmsr = bcm1250MacEthMiiRead (pDrvCtrl, pDrvCtrl->phyAddr, MII_BMSR);
    bmcr = bcm1250MacEthMiiRead (pDrvCtrl, pDrvCtrl->phyAddr, MII_BMCR);

    /* get the link partner status */

    anlpar = bcm1250MacEthMiiRead (pDrvCtrl, pDrvCtrl->phyAddr, MII_ANLPAR);

    /* if supported, read the 1000baseT register */

    if (bmsr & BMSR_1000BT_XSR)
        {
        k1stsr = bcm1250MacEthMiiRead (pDrvCtrl, pDrvCtrl->phyAddr, MII_K1STSR);
        }
    else
        {
        k1stsr = 0;
        }

    (void)printf ("Link speed: ");

    if (k1stsr & K1STSR_LP1KFD)
        {
        pDrvCtrl->macSpeed = MAC_SPEED_1000;
        pDrvCtrl->macDuplex = MAC_DUPLEX_FULL;
        pDrvCtrl->macFc = MAC_FC_FRAME;
        (void)printf ("1000BaseT FDX\n");
        }
    else if (k1stsr & K1STSR_LP1KHD)
        {
        pDrvCtrl->macSpeed = MAC_SPEED_1000;
        pDrvCtrl->macDuplex = MAC_DUPLEX_HALF;
        pDrvCtrl->macFc = MAC_FC_DISABLED;
        (void)printf ("1000BaseT HDX\n");
        }
    else if (anlpar & ANLPAR_TXFD)
        {
        pDrvCtrl->macSpeed = MAC_SPEED_100;
        pDrvCtrl->macDuplex = MAC_DUPLEX_FULL;
        pDrvCtrl->macFc = (anlpar & ANLPAR_PAUSE) ?
                           MAC_FC_FRAME : MAC_FC_DISABLED;
        (void)printf ("100BaseT FDX\n");
        }
    else if (anlpar & ANLPAR_TXHD)
        {
        pDrvCtrl->macSpeed = MAC_SPEED_100;
        pDrvCtrl->macDuplex = MAC_DUPLEX_HALF;
        pDrvCtrl->macFc = MAC_FC_DISABLED;
        (void)printf ("100BaseT HDX\n");
        }
    else if (anlpar & ANLPAR_10FD)
        {
        pDrvCtrl->macSpeed = MAC_SPEED_10;
        pDrvCtrl->macDuplex = MAC_DUPLEX_FULL;
        pDrvCtrl->macFc = MAC_FC_FRAME;
        (void)printf ("10BaseT FDX\n");
        }
    else if (anlpar & ANLPAR_10HD)
        {
        pDrvCtrl->macSpeed = MAC_SPEED_10;
        pDrvCtrl->macDuplex = MAC_DUPLEX_HALF;
        pDrvCtrl->macFc = MAC_FC_COLLISION;
        (void)printf ("10BaseT HDX\n");
        }
    else
        {
        (void)printf ("Unknown\n");
        }

    (void)printf ("BMSR=%04X, BMCR=%04X, 1KSTSR=%04X, ANLPAR=%04X\n",
                  bmsr, bmcr, k1stsr, anlpar);
    }


/*******************************************************************************
*
* bcm1250MacRxDmaShow - display RX DMA register values
*
* This routine prints the enet RX DMA registers to stdout.
*
* RETURNS: N/A
*/

void bcm1250MacRxDmaShow
    (
    int     inst   /* driver instance */
    )
    {
    DRV_CTRL *      pDrvCtrl;   /* driver control structure */
    ETH_MAC_DMA *   pDma;       /* ethernet DMA structure */
    UINT64          reg;        /* register value */
    UINT32          regLo;      /* 32 LSB of register */
    UINT32          regHi;      /* 32 MSB of register */

    pDrvCtrl = (DRV_CTRL *)endFindByName ("sbe", inst);
    if (pDrvCtrl == (DRV_CTRL *)NULL)
        {
        (void)printf ("can't find sbe%d\n", inst);
        return;
        }

    pDma = &pDrvCtrl->rxDma;

    reg = ETH_DMA_REG_READ (pDma->regConfig0);
    regHi = (UINT32)(reg >> 32);
    regLo = (UINT32)(reg & 0xFFFFFFFF);
    (void)printf ("RX0 config0 = 0x%08x%08x\n", regHi, regLo);

    reg = ETH_DMA_REG_READ (pDma->regConfig1);
    regHi = (UINT32)(reg >> 32);
    regLo = (UINT32)(reg & 0xFFFFFFFF);
    (void)printf ("RX0 config1 = 0x%08x%08x\n", regHi, regLo);

    reg = ETH_DMA_REG_READ (pDma->regDscrCnt);
    regHi = (UINT32)(reg >> 32);
    regLo = (UINT32)(reg & 0xFFFFFFFF);
    (void)printf ("RX0 dscrcnt = 0x%08x%08x\n", regHi, regLo);

    reg = ETH_DMA_REG_READ (pDma->regDscrBase);
    regHi = (UINT32)(reg >> 32);
    regLo = (UINT32)(reg & 0xFFFFFFFF);
    (void)printf ("RX0 dscrbase = 0x%08x%08x\n", regHi, regLo);

    reg = ETH_DMA_REG_READ (pDma->regCurDscr);
    regHi = (UINT32)(reg >> 32);
    regLo = (UINT32)(reg & 0xFFFFFFFF);
    (void)printf ("RX0 curdscr = 0x%08x%08x\n", regHi, regLo);

    (void)printf ("RX0 headIndex = %d\n", pDma->headIndex);
    (void)printf ("RX0 tailIndex = %d\n", pDma->tailIndex);
    (void)printf ("RX0 ringCount = %d\n", pDma->ringCount);
    (void)printf ("RX0 maxDescr = %d\n", pDma->maxDescr);
    }


/*******************************************************************************
*
* bcm1250MacTxDmaShow - display TX DMA register values
*
* This routine prints the enet TX DMA registers to stdout.
*
* RETURNS: N/A
*/

void bcm1250MacTxDmaShow
    (
    int     inst    /* driver instance */
    )
    {
    DRV_CTRL *      pDrvCtrl;   /* driver control structure */
    ETH_MAC_DMA *   pDma;       /* ethernet DMA structure */
    UINT64          reg;        /* register value */
    UINT32          regLo;      /* 32 LSB of register */
    UINT32          regHi;      /* 32 MSB of register */

    pDrvCtrl = (DRV_CTRL *)endFindByName ("sbe", inst);
    if (pDrvCtrl == (DRV_CTRL *)NULL)
        {
        (void)printf ("can't find sbe%d\n", inst);
        return;
        }

    pDma = &pDrvCtrl->txDma;

    reg = ETH_DMA_REG_READ (pDma->regConfig0);
    regHi = (UINT32)(reg >> 32);
    regLo = (UINT32)(reg & 0xFFFFFFFF);
    (void)printf ("TX0 config0 = 0x%08x%08x\n", regHi, regLo);

    reg = ETH_DMA_REG_READ (pDma->regConfig1);
    regHi = (UINT32)(reg >> 32);
    regLo = (UINT32)(reg & 0xFFFFFFFF);
    (void)printf ("TX0 config1 = 0x%08x%08x\n", regHi, regLo);

    reg = ETH_DMA_REG_READ (pDma->regDscrCnt);
    regHi = (UINT32)(reg >> 32);
    regLo = (UINT32)(reg & 0xFFFFFFFF);
    (void)printf ("TX0 dscrcnt = 0x%08x%08x\n", regHi, regLo);

    reg = ETH_DMA_REG_READ (pDma->regDscrBase);
    regHi = (UINT32)(reg >> 32);
    regLo = (UINT32)(reg & 0xFFFFFFFF);
    (void)printf ("TX0 dscrbase = 0x%08x%08x\n", regHi, regLo);

    reg = ETH_DMA_REG_READ (pDma->regCurDscr);
    regHi = (UINT32)(reg >> 32);
    regLo = (UINT32)(reg & 0xFFFFFFFF);
    (void)printf ("TX0 curdscr = 0x%08x%08x\n", regHi, regLo);

    (void)printf ("TX0 headIndex = %d\n", pDma->headIndex);
    (void)printf ("TX0 tailIndex = %d\n", pDma->tailIndex);
    (void)printf ("TX0 ringCount = %d\n", pDma->ringCount);
    (void)printf ("TX0 maxDescr = %d\n", pDma->maxDescr);
    }


/*******************************************************************************
*
* bcm1250MacShow - display the MAC register values
*
* This routine prints the enet MAC registers to stdout.
*
* RETURNS: N/A
*/

void bcm1250MacShow
    (
    int     inst    /* driver instance */
    )
    {
    DRV_CTRL *  pDrvCtrl;   /* driver control structure */
    UINT64      reg;        /* register value */
    UINT32      regLo;      /* 32 LSB of register */
    UINT32      regHi;      /* 32 MSB of register */

    pDrvCtrl = (DRV_CTRL *)endFindByName ("sbe", inst);
    if (pDrvCtrl == (DRV_CTRL *)NULL)
        {
        (void)printf ("can't find sbe%d\n", inst);
        return;
        }

    reg = ETH_MAC_REG_READ (pDrvCtrl->regMacEnable);
    regHi = (UINT32)(reg >> 32);
    regLo = (UINT32)(reg & 0xFFFFFFFF);
    (void)printf ("mac macenable = 0x%08x%08x\n", regHi, regLo);

    reg = ETH_MAC_REG_READ (pDrvCtrl->regMacCfg);
    regHi = (UINT32)(reg >> 32);
    regLo = (UINT32)(reg & 0xFFFFFFFF);
    (void)printf ("mac maccfg = 0x%08x%08x\n",  regHi, regLo);

    reg = ETH_MAC_REG_READ (pDrvCtrl->regFifoCfg);
    regHi = (UINT32)(reg >> 32);
    regLo = (UINT32)(reg & 0xFFFFFFFF);
    (void)printf ("mac fifocfg = 0x%08x%08x\n", regHi, regLo);

    reg = ETH_MAC_REG_READ (pDrvCtrl->regFrameCfg);
    regHi = (UINT32)(reg >> 32);
    regLo = (UINT32)(reg & 0xFFFFFFFF);
    (void)printf ("mac framecfg = 0x%08x%08x\n", regHi, regLo);

    reg = ETH_MAC_REG_READ (pDrvCtrl->regRxFilter);
    regHi = (UINT32)(reg >> 32);
    regLo = (UINT32)(reg & 0xFFFFFFFF);
    (void)printf ("mac rxfilter = 0x%08x%08x\n", regHi, regLo);

    reg = ETH_MAC_REG_READ (pDrvCtrl->regImr);
    regHi = (UINT32)(reg >> 32);
    regLo = (UINT32)(reg & 0xFFFFFFFF);
    (void)printf ("mac imr = 0x%08x%08x\n", regHi, regLo);

    reg = ETH_MAC_REG_READ (pDrvCtrl->regIsr);
    regHi = (UINT32)(reg >> 32);
    regLo = (UINT32)(reg & 0xFFFFFFFF);
    (void)printf ("mac isr = 0x%08x%08x\n", regHi, regLo);
    }


/*******************************************************************************
*
* bcm1250MacPhyShow - display the physical register values
*
* This routine prints the enet PHY registers to stdout.
*
* RETURNS: N/A
*/

void bcm1250MacPhyShow
    (
    int     inst    /* driver instance */
    )
    {
    DRV_CTRL *  pDrvCtrl;   /* driver control structure */

    pDrvCtrl = (DRV_CTRL *)endFindByName ("sbe", inst);
    if (pDrvCtrl == (DRV_CTRL *)NULL)
        {
        (void)printf ("can't find sbe%d\n", inst);
        return;
        }

    (void)printf ("PhyReg %02X val %04X\n", 0,
           bcm1250MacEthMiiRead (pDrvCtrl, 1/*pDrvCtrl->phyAddr*/, 0));
    (void)printf ("PhyReg %02X val %04X\n", 0,
           bcm1250MacEthMiiRead (pDrvCtrl, 1/*pDrvCtrl->phyAddr*/, 1));
    }

