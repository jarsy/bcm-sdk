/* endLib.c - support library for END-based drivers */

/* Copyright 1984 - 2005 Wind River Systems, Inc.  */

/* $Id: endLib.c,v 1.3 2011/07/21 16:14:49 yshtil Exp $
modification history
--------------------
02m,08aug05,wap  Put etherbroadcastaddr here to break dependency on the stack
02l,20jan05,vvv  osdep.h cleanup
02k,19oct04,mdo  SPR #102737 - fix dereference of possible null ptr.
02j,04oct04,wap  fix polling stats code for base6 (SPR #101616)
02i,23sep04,ann  changes from networking: fixes for warnings and SPR #100177,
                 SPR #94750.
02h,31aug04,mdo  Documentation fixes for apigen
02g,14jun04,vvv  merged from base6_itn5_networking-dev (SPR #98070)
02f,05may04,vvv  free txSem on unload (SPR #94749)
02e,22apr04,rae  endM2Packet_1213 should be _endM2Packet_1213
02d,16apr04,rae  Add endM2Init() etc.
02c,24nov03,wap  Merge in changes from Snowflake
02b,27sep03,rp   removing WR_IPV6 guard
02a,14jan03,m_h  IPv6 changes
02b,23jan03,vvv  docs: fixed endPollStatsInit markup
02a,13jan03,vvv  merged from branch wrn_velocecp, ver02c (SPR #83251, 82746)
01z,21jun02,wap  avoid bcopy() in endEtherAddressForm()
01y,21may02,vvv  updated mib2ErrorAdd doc (SPR #75064)
01x,17may02,rcs  added endMibIfInit() for initializing the MIB-II callback
                 pointers. SPR# 77478   
01w,26oct01,brk  added cast to stop compile warnings (SPR #65332)
01v,27jun01,rcs  Merg tor2.0.2 to Tornado-Comp-Drv
01u,06feb01,spm  fixed detection of 802.3 Ethernet packets (SPR #27844)
01t,16oct00,spm  merged version 01u from tor3_0_x branch (base version 01r):
         adds support for link-level broadcasts and multiple snarf
         protocols, and code cleanup; removes etherLib.h dependency
01s,29apr99,pul  Upgraded NPT phase3 code to tor2.0.0
01r,16mar99,spm  recovered orphaned code from tor2_0_x branch (SPR #25770)
01q,09dec98,n_s  fixed endEtherPacketDataGet to handle M_BLK chains. spr 23895
01p,18nov98,n_s  fixed end8023AddressForm. spr 22976.
01o,10dec97,gnn  made minor man page fixes
01n,08dec97,gnn  END code review fixes.
01m,17oct97,vin  changed prototypes, fixed endEtherPacketDataGet.
01k,25sep97,gnn  SENS beta feedback fixes
01j,27aug97,gnn  doc fixes and clean up.
01i,25aug97,vin  added netMblkDup in endEtherPacketDataGet().
01h,22aug97,gnn  cleaned up and improved endEtherAddrGet.
01g,19aug97,gnn  changes due to new buffering scheme.
         added routine to get addresses from a packet.
01f,12aug97,gnn  changes necessitated by MUX/END update.
01e,jul3197,kbw  fixed man page problems found in beta review
01d,26feb97,gnn  Set the type of a returned M_BLK_ID correctly.
01c,05feb97,gnn  Added a check for NULL on allocation.
01b,03feb97,gnn  Added default memory management routines.
01a,26dec96,gnn  written.

*/

/*
DESCRIPTION
This library contains support routines for Enhanced Network Drivers.
These routines are common to ALL ENDs.  Specialized routines should only
appear in the drivers themselves.

To use this feature, include the following component:
INCLUDE_END

INCLUDE FILES:
*/

/* includes */

#include "vxWorks.h"
#include "memLib.h"
#include "stdlib.h"
#include "memPartLib.h"
#include "netBufLib.h"
#include "endLib.h"
#include "end.h"
#include "etherMultiLib.h"
#include "bufLib.h"
#include "netinet/if_ether.h"
#include "netLib.h"
#include "wdLib.h"
#include "m2IfLib.h"
#include "muxLib.h"
#include "private/muxLibP.h"

/* defines */

#define LLC_SNAP_FRAMELEN    8

/* typedefs */

/* forward declarations */

STATUS _endM2Packet_1213 (END_OBJ *, M_BLK_ID, UINT);
STATUS _endM2Packet_2233 (END_OBJ *, M_BLK_ID, UINT);

/* globals */

MIB_ROUTINES *  pMibRtn;    /* structure to hold MIB-II callback pointers */ 
FUNCPTR endM2Packet = _endM2Packet_1213;  /* default.  can be set to 2233 */
u_char  etherbroadcastaddr[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

/* locals */

LOCAL void   endPollStatsJobQueue (END_IFDRVCONF *);
LOCAL void   endPollStatsPoll (END_IFDRVCONF *);


void endRcvRtnCall
    (
    END_OBJ* pEnd,
    M_BLK_ID pData
    )
    {
    if (pEnd->receiveRtn)
        {
        pEnd->receiveRtn ((pEnd), pData);
        }
    else
        netMblkClChainFree (pData);
    }

void tkRcvRtnCall
    (
    END_OBJ* pEnd,
    M_BLK_ID pData,
    long netSvcOffset,
    long netSvcType,
    BOOL wrongDstInt,
    void* pAsyncData
    )
    {
    if (pEnd->receiveRtn)
        {
        pEnd->receiveRtn ((pEnd), pData, netSvcOffset , netSvcType, 
                          wrongDstInt, pAsyncData);
        }
    else
        netMblkClChainFree (pData);
    }


void endTxSemTake
    (
    END_OBJ* pEnd,
    int tmout 
    ) 
    {
    semTake(pEnd->txSem,tmout);
    }

void endTxSemGive
    (
    END_OBJ* pEnd
    )
    {
    semGive (pEnd->txSem);
    }

void endFlagsClr
    (
    END_OBJ* pEnd,
    int clrBits 
    ) 
    {
    pEnd->flags &= ~clrBits;
    }

void endFlagsSet
    (
    END_OBJ* pEnd,
    int setBits
    )
    {
    pEnd->flags |= (setBits);
    }

int endFlagsGet
    (
    END_OBJ* pEnd
    ) 
    {
    return(pEnd->flags);
    }

int endMultiLstCnt
    (
    END_OBJ* pEnd
    )
    {
    return(lstCount(&pEnd->multiList));
    }

ETHER_MULTI* endMultiLstFirst
    (
    END_OBJ *pEnd
    ) 
    {
    return((ETHER_MULTI *)(lstFirst(&pEnd->multiList)));
    }

ETHER_MULTI* endMultiLstNext
    (
    ETHER_MULTI* pCurrent
    ) 
    {
    return((ETHER_MULTI *)(lstNext(&pCurrent->node)));
    }

char* endDevName
    (
    END_OBJ* pEnd
    ) 
    {
    return(pEnd->devObject.name);
    }

void endObjectUnload
    (
    END_OBJ* pEnd
    )
    {
    if (pEnd->txSem != NULL)
        semDelete (pEnd->txSem);

    lstFree (&pEnd->multiList); 
    if (pEnd->pSnarf != NULL)
        {
        KHEAP_FREE (pEnd->pSnarf);
        }

    pEnd->dummyBinding = NULL;

    pEnd->pSnarf = NULL;
    pEnd->pTyped = NULL;
    pEnd->pPromisc = NULL;
    pEnd->pStop = NULL;
    pEnd->nProtoSlots = 0;
    }


/******************************************************************************
*
* endM2Init - Initialize MIB-II agnostically (1213 or 2233)
*
* This routine encapsulates and generalizes mib2Init and endObjFlagSet
*
* RETURNS: OK or ERROR.
*
* ERRNO
*
* \NOMANUAL
*/

STATUS endM2Init
    (
    END_OBJ *   pEndObj,        /* object to be initialized */
    long        ifType,         /* ifType from m2Lib.h */
    UCHAR *     phyAddr,        /* MAC/PHY address */
    int         addrLength,     /* MAC/PHY address length */
    int         mtuSize,        /* MTU size */
    int         speed,          /* interface speed */
    UINT        flags           /* flags to set in the pEndObj */
    )
    {
    M2_INTERFACETBL *pM2Int;    /* 1213 struct to be initialized */
    M2_ID **ppM2Id;             /* 2233 struct to be initialized */
    int status;
    
 
    if (pMibRtn == NULL)                     /* 1213? */
        {
        pM2Int = &(pEndObj->mib2Tbl);
        status = mib2Init (pM2Int, ifType, phyAddr,
                           addrLength, mtuSize, speed);
        }
        
    else                                     /* 2233? */
        {
        ppM2Id = &(pEndObj->pMib2Tbl);
        status = (pMibRtn->mibAlloc) (ppM2Id, ifType, phyAddr,
                                      addrLength, mtuSize, speed,
                                      pEndObj->devObject.name,
                                      pEndObj->devObject.unit);
        endM2Packet = _endM2Packet_2233;
        flags |=  END_MIB_2233;

        /* backwards compatibility */
        bcopy ((char *)&(pEndObj->pMib2Tbl->m2Data.mibIfTbl),
               (char *)&(pEndObj->mib2Tbl), sizeof (M2_INTERFACETBL));
        }

    endObjFlagSet (pEndObj, flags);
    return status;
    }

/******************************************************************************
*
* endM2Free - Frees what endM2Init allocates
*
* This routine determines what endM2Init created, and frees it
*
* RETURNS: OK or ERROR.
*
* ERRNO
*
* \NOMANUAL
*/

STATUS endM2Free
    (
    END_OBJ *   pEndObj        /* object where m2 data lives */
    )
    {
    if (pMibRtn == NULL)                     /* 1213? */
        return OK;
    else                                     /* 2233 */
        return (pMibRtn->mibFree) (pEndObj->pMib2Tbl);
    }

/******************************************************************************
*
* endM2Ioctl - generic ioctl
*
* This routine handles the EIOCGMIB2 and EIOCGMIB2233 ioctls.
*
* RETURNS: OK or ERROR.
*
* ERRNO: EINVAL
*
* \NOMANUAL
*/

int endM2Ioctl
    (
    END_OBJ *   pEndObj,      /* object where m2 data lives */
    UINT32      cmd,          /* command to process */
    caddr_t     data          /* pointer to data */
    )
    {
    int                error = OK;
 
    switch (cmd)
        {
        case EIOCGMIB2:
            if (data == NULL)
                error=EINVAL;
            else
                bcopy ((char *) &pEndObj->mib2Tbl, (char *) data,
                        sizeof (pEndObj->mib2Tbl));
 
            break;
 
        case EIOCGMIB2233:
            if ((data == NULL) || (pEndObj->pMib2Tbl == NULL))
                error = EINVAL;
            else
                *((M2_ID **)data) = pEndObj->pMib2Tbl;
            break;
        default:
            error = EINVAL;
            break;
        }
    return (error);
    }
           
/******************************************************************************
*
* _endM2Packet_1213 - count a packet assuming 1213
*
* This routine counts a packet in the absence of 2233.
*
* RETURNS: OK or ERROR.
*
* ERRNO
*
* \NOMANUAL
*/

STATUS _endM2Packet_1213
    (
    END_OBJ *   pEndObj,   /* object where counter lives */
    M_BLK_ID    pMblk,     /* packet to count */
    UINT        ctrl       /* M2_PACKET_IN OUT, etc. */
    )
    {
    M2_INTERFACETBL *pMib = &(pEndObj->mib2Tbl);   /* struct to be updated */
    BOOL isNUcast = 0;                             /* is the packet non-unicast? */
    int status = OK;
    int nMACOffset = 0;

    /* we know that the destination address is the first byte in the frame */
    if (pMblk != NULL)
        {
        /*
         * Special handling for 802.4 and 802.5. In 802.4,
         * the destination address is preceded by a single
         * frame control byte. In 802.5, the destination
         * address is preceded by an access control byte
         * and a frame control byte.
         */

        if (pMib->ifType == M2_ifType_iso88024_tokenBus)
            nMACOffset = 1;

        if (pMib->ifType == M2_ifType_iso88025_tokenRing)
            nMACOffset = 2;

        isNUcast = (pMblk->mBlkHdr.mData[nMACOffset] & (UINT8) 0x01);
        }
    else
        /* Don't go on */
        return (ERROR);

    switch (ctrl)
        {
        case M2_PACKET_IN_ERROR:
            pMib->ifInErrors++;
            break;

        case M2_PACKET_IN:
            if (isNUcast)
                pMib->ifInNUcastPkts++;
            else
                pMib->ifInUcastPkts++;
            pMib->ifInOctets += pMblk->mBlkPktHdr.len;
            break;

        case M2_PACKET_OUT_ERROR:
            pMib->ifOutErrors++;
            break;

        case M2_PACKET_OUT:
            if (isNUcast)
                pMib->ifOutNUcastPkts++;
            else
                pMib->ifOutUcastPkts++;
            pMib->ifOutOctets += pMblk->mBlkHdr.mLen;
            break;

        case M2_PACKET_IN_DISCARD:
            pMib->ifInDiscards++;
            break;

        case M2_PACKET_OUT_DISCARD:
            pMib->ifOutDiscards++;
            break;

        default:
            status = ERROR;
            break;         
        }
    return status;
    }
    
/******************************************************************************
*
* _endM2Packet_2233 - count a packet assuming 2233
*
* This routine counts a packet in the presence of 2233.
*
* RETURNS: OK or ERROR.
*
* ERRNO
*
* \NOMANUAL
*/

STATUS _endM2Packet_2233
    (
    END_OBJ *   pEndObj,   /* object where counter lives */
    M_BLK_ID    pMblk,     /* packet to count */
    UINT        ctrl       /* M2_PACKET_IN OUT, etc. */
    )
    {
    int status = OK;
    M2_ID * pMib2Tbl;           /* RFC 2233 MIB objects */
    pMib2Tbl = pEndObj->pMib2Tbl;

    switch (ctrl)
        {
        case M2_PACKET_IN_ERROR:
            status = (pMib2Tbl->m2CtrUpdateRtn) (pMib2Tbl,
                                                 M2_ctrId_ifInErrors, 1);
            break;

        case M2_PACKET_IN:
        case M2_PACKET_OUT:
            status = (pMib2Tbl->m2PktCountRtn) (pMib2Tbl, ctrl,
                                                (UCHAR *) pMblk->m_data, 
                                                (ULONG) pMblk->m_pkthdr.len);
            break;

        case M2_PACKET_OUT_ERROR:
            status = (pMib2Tbl->m2CtrUpdateRtn) (pMib2Tbl,
                                                 M2_ctrId_ifOutErrors, 1);
            break;

        case M2_PACKET_IN_DISCARD:
            status = (pMib2Tbl->m2CtrUpdateRtn) (pMib2Tbl,
                                                 M2_ctrId_ifInDiscards, 1);  
            break;

        case M2_PACKET_OUT_DISCARD:
            status = (pMib2Tbl->m2CtrUpdateRtn) (pMib2Tbl,
                                                 M2_ctrId_ifOutDiscards, 1); 
            break;

        default:
            status = ERROR;
            break;         

        }
    
    return status;
    }

/******************************************************************************
*
* endMibIfInit - Initialize the MIB-II callback pointers
*
* This routine initializes the MIB-II pointers used by the callback macros.
*
* RETURNS: OK or ERROR.
*
* ERRNO
*
* \NOMANUAL
*/

STATUS endMibIfInit
    (
    FUNCPTR pMibAllocRtn,
    FUNCPTR pMibFreeRtn,
    FUNCPTR pMibCtrUpdate,
    FUNCPTR pMibVarUpdate
    )
    {
    if ((pMibAllocRtn != NULL) && (pMibFreeRtn != NULL) && 
        (pMibCtrUpdate != NULL) && (pMibVarUpdate != NULL))
        {
        if (pMibRtn == NULL)
            {
            if ((pMibRtn = 
                 (MIB_ROUTINES *) KHEAP_ALLOC (sizeof (MIB_ROUTINES))) == NULL)
                {
                return(ERROR);
                }
            }
        pMibRtn->mibAlloc = (FUNCPTR)pMibAllocRtn;
        pMibRtn->mibFree = (FUNCPTR)pMibFreeRtn;
        pMibRtn->mibCntUpdate = (FUNCPTR)pMibCtrUpdate;
        pMibRtn->mibVarUpdate = (FUNCPTR)pMibVarUpdate;

        return(OK);
        }
    else
        {
        return(ERROR);
        }
    }

/******************************************************************************
*
* mib2Init - initialize a MIB-II structure
*
* Initialize a MIB-II structure.  Set all error counts to zero.  Assume
* a 10Mbps Ethernet device.
*
* RETURNS: OK or ERROR.
*
* ERRNO
*/

STATUS mib2Init
    (
    M2_INTERFACETBL *pMib,      /* struct to be initialized */
    long ifType,                /* ifType from m2Lib.h */
    UCHAR * phyAddr,            /* MAC/PHY address */
    int addrLength,             /* MAC/PHY address length */
    int mtuSize,                /* MTU size */
    int speed                   /* interface speed */
    )
    {
    /* Clear out our statistics. */

    bzero ((char *)pMib, sizeof (*pMib));

    pMib->ifPhysAddress.addrLength = addrLength;

    /* Obtain our Ethernet address and save it */
    bcopy ((char *) phyAddr, (char *)pMib->ifPhysAddress.phyAddress,
           pMib->ifPhysAddress.addrLength);

    /* Set static statistics. assume ethernet */

    pMib->ifType = ifType;
    pMib->ifMtu =  mtuSize;
    pMib->ifSpeed = speed;

    return OK;
    }

/******************************************************************************
*
* mib2ErrorAdd - change a MIB-II error count
*
* This function adds a specified value to one of the MIB-II error counters in a 
* MIB-II interface table.  The counter to be altered is specified by the 
* errCode argument. errCode can be MIB2_IN_ERRS, MIB2_IN_UCAST, MIB2_OUT_ERRS 
* or MIB2_OUT_UCAST. Specifying a negative value reduces the error count, a 
* positive value increases the error count.
*
* RETURNS: OK 
*
* ERRNO
*/

STATUS mib2ErrorAdd
    (
    M2_INTERFACETBL * pMib,
    int errCode,
    int value
    )
    {
    
    switch (errCode)
        {
        default:
        case MIB2_IN_ERRS:
            pMib->ifInErrors += value;
            break;

        case MIB2_IN_UCAST:
            pMib->ifInUcastPkts += value;
            break;

        case MIB2_OUT_ERRS:
            pMib->ifOutErrors += value;
            break;

        case MIB2_OUT_UCAST:
            pMib->ifOutUcastPkts += value;
            break;
        }

    return OK;
    }

/*******************************************************************************
*
* endObjInit - initialize an END_OBJ structure
*
* This routine initializes an END_OBJ structure and fills it with data from 
* the argument list.  It also creates and initializes semaphores and 
* protocol list.
*
* RETURNS: OK or ERROR.
*
* ERRNO
*/

STATUS endObjInit
    (
    END_OBJ *   pEndObj,        /* object to be initialized */
    DEV_OBJ*    pDevice,        /* ptr to device struct */
    char *      pBaseName,      /* device base name, for example, "ln" */
    int         unit,           /* unit number */
    NET_FUNCS * pFuncTable,     /* END device functions */
    char*       pDescription
    )
    {
    int maxLen;

    pEndObj->devObject.pDevice = pDevice;

    /* Create the transmit semaphore. */

    pEndObj->txSem = semMCreate ( SEM_Q_PRIORITY  |
                                  SEM_DELETE_SAFE |
                                  SEM_INVERSION_SAFE);

    if (pEndObj->txSem == NULL)
        {
        return (ERROR);
        }

    /* Install data and functions into the network node. */

    pEndObj->flags = 0;

    /* initialize protocol table (empty) */

    pEndObj->pSnarf   = NULL;
    pEndObj->pTyped   = NULL;
    pEndObj->pPromisc = NULL;
    pEndObj->pStop    = NULL;
    pEndObj->nProtoSlots = 0;

    /* Don't modify the strings passed as arguments! */

    maxLen = sizeof(pEndObj->devObject.name) - 1;

    strncpy (pEndObj->devObject.name, pBaseName, maxLen);
    pEndObj->devObject.name [maxLen] = EOS;

    maxLen = sizeof(pEndObj->devObject.description) - 1;

    strncpy (pEndObj->devObject.description, pDescription, maxLen);
    pEndObj->devObject.description [maxLen] = EOS;

    pEndObj->devObject.unit = unit;
    
    pEndObj->pFuncTable = pFuncTable;

    /* Clear multicast info. */

    lstInit (&pEndObj->multiList);
    pEndObj->nMulti = 0;

    pEndObj->dummyBinding = pEndObj;

    return OK;
    }

/*******************************************************************************
*
* endObjFlagSet - set the `flags' member of an END_OBJ structure
*
* As input, this routine expects a pointer to an END_OBJ structure 
* (the <pEnd> parameter) and a flags value (the <flags> parameter).
* This routine sets the 'flags' member of the END_OBJ structure
* to the value of the <flags> parameter. 
*
* Because this routine assumes that the driver interface is now up,  
* this routine also sets the 'attached' member of the referenced END_OBJ
* structure to TRUE. 
*
* RETURNS: OK
*
* ERRNO
*/

STATUS endObjFlagSet
    (
    END_OBJ * pEnd,
    UINT        flags
    )
    {
    pEnd->attached = TRUE;
    pEnd->flags = flags;

    return OK;
    }

/******************************************************************************
*
* end8023AddressForm - form an 802.3 address into a packet
*
* This routine accepts the source and destination addressing information
* through <pSrcAddr> and <pDstAddr> mBlks and returns an M_BLK_ID to the
* assembled link level header.  If the <bcastFlag> argument is TRUE, it
* sets the destination address to the link-level broadcast address and
* ignores the <pDstAddr> contents. This routine prepends the link level header
* into <pMblk> if there is enough space available or it allocates a new
* mBlk/clBlk/cluster and prepends the new mBlk to the mBlk chain passed in
* <pMblk>.  This routine returns a pointer to an mBlk which contains the
* link level header information.
*
* RETURNS: M_BLK_ID or NULL.
*
* ERRNO
*
* \NOMANUAL
*/

M_BLK_ID end8023AddressForm
    (
    M_BLK_ID pMblk,
    M_BLK_ID pSrcAddr,
    M_BLK_ID pDstAddr,
    BOOL bcastFlag
    )
    {
    short dataLen;     /* length of data including LLC */
    USHORT *pSrc;
    USHORT *pDst;

    struct llc * pLlc; /* link layer control header */

    dataLen = pMblk->mBlkPktHdr.len + LLC_SNAP_FRAMELEN;
    dataLen = htons (dataLen);
 
    M_PREPEND(pMblk, SIZEOF_ETHERHEADER + LLC_SNAP_FRAMELEN, M_DONTWAIT);

    if (pMblk != NULL)
        {

        /* Fill in destination address */

        pDst = (USHORT *)pMblk->m_data;
        if (bcastFlag)
            {
            pDst[0] = 0xFFFF;
            pDst[1] = 0xFFFF;
            pDst[2] = 0xFFFF;
            }
        else
            {
            pSrc = (USHORT *)pDstAddr->m_data;
            pDst[0] = pSrc[0];
            pDst[1] = pSrc[1];
            pDst[2] = pSrc[2];
            }

        /* Fill in source address */

        pDst += 3;
        pSrc = (USHORT *)pSrcAddr->m_data;
        pDst[0] = pSrc[0];
        pDst[1] = pSrc[1];
        pDst[2] = pSrc[2];

        /* Fill in length */

        pDst += 3;
        pDst[0] = dataLen;

        /* Fill in LLC using SNAP values */

        pDst++;
        pLlc = (struct llc *) pDst;

        pLlc->llc_dsap = LLC_SNAP_LSAP;
        pLlc->llc_ssap = LLC_SNAP_LSAP; 
        pLlc->llc_un.type_snap.control = LLC_UI;
        pLlc->llc_un.type_snap.org_code[0] =
        pLlc->llc_un.type_snap.org_code[1] =
        pLlc->llc_un.type_snap.org_code[2] = 0;

        /* Enter ethernet network type code into the LLC snap field */

        pLlc->llc_un.type_snap.ether_type = pDstAddr->mBlkHdr.reserved;

        }
    
    return(pMblk);
    }

/******************************************************************************
*
* endEtherAddressForm - form an Ethernet address into a packet
*
* This routine accepts the source and destination addressing information 
* through <pSrcAddr> and <pDstAddr> and returns an 'M_BLK_ID' that points 
* to the assembled link-level header.  To do this, this routine prepends 
* the link-level header into the cluster associated with <pMblk> if there 
* is enough space available in the cluster.  It then returns a pointer to 
* the pointer referenced in <pMblk>.  However, if there is not enough space 
* in the cluster associated with <pMblk>, this routine reserves a 
* new 'mBlk'-'clBlk'-cluster construct for the header information. 
* It then prepends the new 'mBlk' to the 'mBlk' passed in <pMblk>.  As the 
* function value, this routine then returns a pointer to the new 'mBlk', 
* which the head of a chain of 'mBlk' structures.  The second element of this 
* chain is the 'mBlk' referenced in <pMblk>. 
*
* RETURNS: M_BLK_ID or NULL.
*
* ERRNO
*/

M_BLK_ID endEtherAddressForm
    (
    M_BLK_ID pMblk,     /* pointer to packet mBlk */
    M_BLK_ID pSrcAddr,  /* pointer to source address */
    M_BLK_ID pDstAddr,  /* pointer to destination address */
    BOOL bcastFlag      /* use link-level broadcast? */
    )
    {
    USHORT *pDst;
    USHORT *pSrc;
    M_PREPEND(pMblk, SIZEOF_ETHERHEADER, M_DONTWAIT);

    /*
     * This routine has been optimized somewhat in order to avoid
     * the use of bcopy(). On some architectures, a bcopy() could
     * result in a call into (allegedly) optimized architecture-
     * specific routines. This may be fine for copying large chunks
     * of data, but we're only copying 6 bytes. It's simpler just
     * to open code some 16-bit assignments. The compiler would be
     * hard-pressed to produce sub-optimal code for this, and it
     * avoids at least one function call (possibly several).
     */

    if (pMblk != NULL)
        {
        pDst = (USHORT *)pMblk->m_data;
        if (bcastFlag)
            {
            pDst[0] = 0xFFFF;
            pDst[1] = 0xFFFF;
            pDst[2] = 0xFFFF;
            }
        else
            {
            pSrc = (USHORT *)pDstAddr->m_data;
            pDst[0] = pSrc[0];
            pDst[1] = pSrc[1];
            pDst[2] = pSrc[2];
            }

        /* Advance to the source address field, fill it in. */
        pDst += 3;
        pSrc = (USHORT *)pSrcAddr->m_data;
        pDst[0] = pSrc[0];
        pDst[1] = pSrc[1];
        pDst[2] = pSrc[2];

        ((struct ether_header *)pMblk->m_data)->ether_type =
        pDstAddr->mBlkHdr.reserved;
        }
    
    return(pMblk);
    }

/******************************************************************************
*
* endEtherPacketDataGet - return the beginning of the packet data
*
* This routine fills the given <pLinkHdrInfo> with the appropriate offsets.
*
* RETURNS: OK or ERROR.
*
* ERRNO
*/

STATUS endEtherPacketDataGet
    (
    M_BLK_ID            pMblk,
    LL_HDR_INFO *       pLinkHdrInfo
    )
    {
    struct ether_header *  pEnetHdr;
    struct ether_header    enetHdr;

    struct llc *           pLLCHdr;
    struct llc             llcHdr;

    USHORT                 etherType;

    pLinkHdrInfo->destAddrOffset        = 0;
    pLinkHdrInfo->destSize              = 6;
    pLinkHdrInfo->srcAddrOffset         = 6;
    pLinkHdrInfo->srcSize               = 6;

    /* Try for RFC 894 first as it's the most common. */

    /* 
     * make sure entire ether_header is in first M_BLK 
     * if not then copy the data to a temporary buffer 
     */

    if (pMblk->mBlkHdr.mLen < SIZEOF_ETHERHEADER)
        {
        pEnetHdr = &enetHdr;
        if (netMblkOffsetToBufCopy (pMblk, 0, (char *) pEnetHdr, 
                                    SIZEOF_ETHERHEADER, (FUNCPTR) bcopy) 
            < SIZEOF_ETHERHEADER)
            {
            return(ERROR);
            }
        }
    else
        pEnetHdr = (struct ether_header *)pMblk->mBlkHdr.mData;

    etherType = ntohs(pEnetHdr->ether_type);

    /* Deal with 802.3 addressing. */

    /* Here is the algorithm. */
    /* If the etherType is less than or equal to the MTU then we know that */
    /* this is an 802.x address from RFC 1700. */
    if (etherType <= ETHERMTU)
        {

        /* 
         * make sure entire ether_header + llc_hdr is in first M_BLK 
         * if not then copy the data to a temporary buffer 
         */

        if (pMblk->mBlkHdr.mLen < SIZEOF_ETHERHEADER + LLC_SNAP_FRAMELEN)
            {
            pLLCHdr = &llcHdr;
            if (netMblkOffsetToBufCopy (pMblk, SIZEOF_ETHERHEADER, 
                                        (char *) pLLCHdr, LLC_SNAP_FRAMELEN,
                                        (FUNCPTR) bcopy)
                < LLC_SNAP_FRAMELEN)
                {
                return(ERROR);
                }
            }
        else
            pLLCHdr = (struct llc *)((char *)pEnetHdr + SIZEOF_ETHERHEADER);

        /* Now it may be IP over 802.x so we check to see if the */
        /* destination SAP is IP, if so we snag the ethertype from the */
        /* proper place. */

        /* Now if it's NOT IP over 802.x then we just used the DSAP as */
        /* the etherType.  */

        if (pLLCHdr->llc_dsap == LLC_SNAP_LSAP)
            {
            etherType = ntohs(pLLCHdr->llc_un.type_snap.ether_type);
            pLinkHdrInfo->dataOffset = SIZEOF_ETHERHEADER + 8;
            }
        else
            { /* no SNAP header */
            pLinkHdrInfo->dataOffset = SIZEOF_ETHERHEADER + 3;
            etherType = pLLCHdr->llc_dsap;
            }
        }
    else
        {
        pLinkHdrInfo->dataOffset        = SIZEOF_ETHERHEADER;
        }
    pLinkHdrInfo->pktType               = etherType;

    return(OK);
    }

/******************************************************************************
*
* endEtherPacketAddrGet - locate the addresses in a packet
*
* This routine takes a 'M_BLK_ID', locates the address information, and 
* adjusts the M_BLK_ID structures referenced in <pSrc>, <pDst>, <pESrc>, 
* and <pEDst> so that their pData members point to the addressing 
* information in the packet.  The addressing information is not copied. 
* All 'mBlk' structures share the same cluster.
*
* RETURNS: OK
*
* ERRNO
*/

STATUS endEtherPacketAddrGet
    (
    M_BLK_ID pMblk, /* pointer to packet */
    M_BLK_ID pSrc,  /* pointer to local source address */
    M_BLK_ID pDst,  /* pointer to local destination address */
    M_BLK_ID pESrc, /* pointer to remote source address (if any) */
    M_BLK_ID pEDst  /* pointer to remote destination address (if any) */
    )
    {

    if (pSrc != NULL)
        {
        pSrc = netMblkDup (pMblk, pSrc);
        pSrc->mBlkHdr.mData += 6;
        pSrc->mBlkHdr.mLen = 6;
        }
    if (pDst != NULL)
        {
        pDst = netMblkDup (pMblk, pDst);
        pDst->mBlkHdr.mLen = 6;
        }
    if (pESrc != NULL)
        {
        pESrc = netMblkDup (pMblk, pESrc);
        pESrc->mBlkHdr.mData += 6;
        pESrc->mBlkHdr.mLen = 6;
        }
    if (pEDst != NULL)
        {
        pEDst = netMblkDup (pMblk, pEDst);
        pEDst->mBlkHdr.mLen = 6;
        }
    
    return (OK);
    }

/******************************************************************************
*
* endPollStatsPoll - poll an interface's information
*
* This routine calls the endPollStatsIfPoll() routine to gather
* information from a particular interface, then re-arms the interface's
* polling watchdog timer.
*
* RETURNS: N/A
*
* ERRNO
*/

LOCAL void endPollStatsPoll
    (
    END_IFDRVCONF   *pDrvConf
    )
    {

    if (pDrvConf == NULL)
        return;

    /* Gather the stats */

    pDrvConf->ifPollRtn (pDrvConf);

    /* Reload the watchdog. */

    wdStart(pDrvConf->ifWatchdog, pDrvConf->ifPollInterval,
            (FUNCPTR) endPollStatsJobQueue, (int) (pDrvConf));

    return;

    }

/******************************************************************************
*
* endPollStatsJobQueue - polling stats counter scheduler routine
*
* This routine is an intermediary function necessitated by the fact
* that wdStart() only takes two arguments: we need to pass it three
* (netJobAdd(), plus two others). This routine just invokes netJobAdd()
* to call the endPollStatsIfPoll() routine and passes it a pointer to
* the END_IFDRVCONF structure being polled. We must use netJobAdd()
* here to avoid executing the rest of the polling code at interrupt
* context.
*
* RETURNS: N/A
*
* ERRNO
*/

LOCAL void endPollStatsJobQueue
    (
    END_IFDRVCONF   *pDrvConf
    )
    {

    if (netJobAdd ((FUNCPTR) endPollStatsPoll, (int) pDrvConf, 0, 0, 0, 0)
        != OK)
        {
        /* Reload the watchdog. */

        wdStart(pDrvConf->ifWatchdog, pDrvConf->ifPollInterval,
                (FUNCPTR) endPollStatsJobQueue, (int) (pDrvConf));

        }

    return;
    }

/******************************************************************************
*
* endPollStatsInit - initialize polling statistics updates
*
* This routine is used to begin polling of the interface specified by
* pCookie and will periodically call out to the pIfPollRtn function, which
* will collect the interface statistics. If the driver supports polling
* updates, this routine will start a watchdog that will invoke the
* pIfPollRtn routine periodically. The watchdog will automatically
* re-arm itself. The pIfPollRtn will be passed a pointer to the driver's
* END_IFDRVCONF structure as an argument.
*
* RETURNS: ERROR if the driver doesn't support polling, otherwise OK.
*
* ERRNO
*/

STATUS endPollStatsInit
    (
    void *      pCookie,
    FUNCPTR     pIfPollRtn
    )
    {
    END_IFDRVCONF * pDrvConf;
    int         error;
    PROTOCOL_BINDING	tmpBinding;

    /* Sanity check */

    if (pCookie == NULL || pIfPollRtn == NULL)
        return (ERROR);

    tmpBinding.pEnd = PDEVCOOKIE_TO_ENDOBJ(pCookie);

    if (tmpBinding.pEnd == NULL)
        return (ERROR);

    /* Get this driver's configuration info */

    error = muxIoctl ((void *)&tmpBinding, EIOCGPOLLCONF,
                      (caddr_t) &pDrvConf);

    /* Return error if driver doesn't support stats polling. */

    if (error)
        return (error);

    /* Create the watchdog. */

    pDrvConf->ifWatchdog = wdCreate ();
    if (pDrvConf->ifWatchdog == NULL)
        return (ERROR);

    pDrvConf->ifEndObj = NULL;
    pDrvConf->ifMuxCookie = pCookie;
    pDrvConf->ifPollRtn = pIfPollRtn;

    /* Kick off the watchdog. */

    wdStart (pDrvConf->ifWatchdog, pDrvConf->ifPollInterval,
             (FUNCPTR) endPollStatsJobQueue, (int) (pDrvConf));

    return (OK);

    }
