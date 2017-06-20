/*
 *
 * =================================================
 * ==  sbQe2000ElibVlan.c - elib VLAN Management  ==
 * =================================================
 *
 * WORKING REVISION: $Id: sbQe2000ElibVlan.c,v 1.8 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * MODULE NAME:
 *
 *     sbQe2000ElibVlan.c
 *
 * ABSTRACT:
 *
 *     elib public API for VLAN Management
 *
 * LANGUAGE:
 *
 *     C
 *
 * AUTHORS:
 *
 *     Travis B. Sawyer
 *
 * CREATION DATE:
 *
 *     21-June-2005
 *
 */
#include "sbWrappers.h"
#include "sbQe2000Elib.h"
#include "sbTypesGlue.h"
#include "sbTypes.h"
#include "sbQe2000ElibMem.h"
#include "sbQe2000ElibContext.h"
#include "sbQe2000ElibZf.h"


/**
 * VLAN Indirection Table (VIT) Entry
 *
 * The VLAN Indirection Table (VIT) is used by the EP Bridging function in order
 * to resolve the bevavior of a given VLAN.  The table is indexed by VLAN, and
 * contains 4K entries.  Each entry in the VIT may be is one of two forms:
 *
 * - Indirection Record
 * - Control Record
 *
 * An Indirection Record points to an entry in the VLAN Resolution Table (VRT).
 * The VRT record contains more detailed configuration information for the
 * VLAN.  A Control Record contains immediate information for the VLAN, and
 * contains an operation code (SB_QE2000_ELIB_VIT_OP_T) that determines the processing
 * of the given VLAN.
 *
 * The bOpValid field determins the type of VIT record.
 *
 * bOpValid = 1 indicates that the VIT is a Control Record and the Op field is
 * valid.
 *
 * bOpValid = 0 indicates that the VIT is an Indirection Record and the ulPtr
 * and Cmap fields are valid.
 *
 * NOTE:  Should ulPtr be calculated internally?  It is used by VRTSet/Get
 */
typedef struct sb_qe2000_elib_vit_s {
    bool_t bOpValid;                 /**< OP field is valid.                       */
    SB_QE2000_ELIB_VIT_OP_T Op;      /**< Control record operation encoding.       */
    uint32 ulPtr;                  /**< Indirection record ptr.                  */
    SB_QE2000_ELIB_VIT_CMAP_T Cmap;  /**< Indirection record counter map encoding. */
} SB_QE2000_ELIB_VIT_ST, *SB_QE2000_ELIB_VIT_PST;

/**
 * Set an entry in the VLAN Indirection Table (VIT)
 *
 * See the documentation for SB_QE2000_ELIB_VIT_ST for details on the structure of the VIT.
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param ulVLAN     The VLAN.  Valid range of 0..SB_QE2000_ELIB_NUM_VLANS_K - 1.
 *
 * @param Entry      The VIT entry.  This is an SB_QE2000_ELIB_VIT_ST struct.  It contains
 *                   the VIT entry information.
 *
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
static sbElibStatus_et sbQe2000ElibVITSet( SB_QE2000_ELIB_CONTEXT_PST pEp, uint32 ulVLAN, SB_QE2000_ELIB_VIT_ST Entry );


/**
 * Get an entry from the VLAN Indirection Table (VIT)
 *
 * See the documentation for SB_QE2000_ELIB_VIT_ST for details on the structure of the VIT.
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param ulVLAN     The VLAN.  Valid range of 0..SB_QE2000_ELIB_NUM_VLANS_K - 1.
 *
 * @param pEntry     The VIT entry.  This is a pointer to an SB_QE2000_ELIB_VIT_ST struct.
 *                   Upon a return indication of success, it contains
 *                   the VIT entry information for the given VLAN.
 *
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
static sbElibStatus_et sbQe2000ElibVITGet( SB_QE2000_ELIB_CONTEXT_PST pEp, uint32 ulVLAN, SB_QE2000_ELIB_VIT_PST pEntry );


static sbElibStatus_et sbQe2000ElibScIdxCountAccumulate( SB_QE2000_ELIB_CONTEXT_PST pEp, uint32 ulScIdx, uint32 *pulCounts );
static int sbQe2000ElibGetPopulationCount(uint64 ullPortEnable);
static uint32 sbQe2000ElibGetPopulationCount32( uint32 uliValue );


static uint32 sbQe2000ElibGetPopulationCount32( uint32 uliValue )
{
  uint32 uliResult = uliValue;

  uliResult = ((uliResult & 0xAAAAAAAA) >> 1)  + (uliResult & 0x55555555);
  uliResult = ((uliResult & 0xCCCCCCCC) >> 2)  + (uliResult & 0x33333333);
  uliResult = ((uliResult & 0xF0F0F0F0) >> 4)  + (uliResult & 0x0F0F0F0F);
  uliResult = ((uliResult & 0xFF00FF00) >> 8)  + (uliResult & 0x00FF00FF);
  uliResult = ((uliResult & 0xFFFF0000) >> 16) + (uliResult & 0x0000FFFF);

  return( uliResult );
}

int sbQe2000ElibGetPopulationCount(uint64 ullPortEnable)
{
    uint32 res0, res1;

    res0 = sbQe2000ElibGetPopulationCount32(COMPILER_64_LO(ullPortEnable));
    res1 = sbQe2000ElibGetPopulationCount32(COMPILER_64_HI(ullPortEnable));
    return( res0 + res1 );

}

static sbElibStatus_et sbQe2000ElibVITSet( SB_QE2000_ELIB_CONTEXT_PST pEp, uint32 ulVLAN, SB_QE2000_ELIB_VIT_ST Entry )
{
    sbZfSbQe2000ElibVIT_t sZfVIT;
    uint32 aulVITTuple[2];
    uint32 ulRecNum;
    uint32 ulVIRNum;
    uint32 ulNewVIR;
    sbElibStatus_et nStatus;

    DENTER();
    SB_ASSERT( pEp );
    SB_ASSERT( ulVLAN < SB_QE2000_ELIB_NUM_VLANS_K );

    /*
     * Figure out the memory location of the VIR, and the
     * VIT tuple
     *
     * 4 entries per mem loc
     *
     * NOTE:  ulVLAN == ScIdx
     */
    ulVIRNum = ulVLAN % 4;
    ulRecNum = ulVLAN / 4;

    /*
     * Get the initial entry tuple (4 records)
     * so we can modify and write it back out
     */
    nStatus = sbQe2000ElibBfMemRead( pEp->pHalCtx,
                                     ulRecNum,
                                     0,
                                     &aulVITTuple[0],
                                     &aulVITTuple[1] );
    if( SB_ELIB_OK != nStatus )
    {
        DEXIT();
        return( nStatus );
    }

    sbZfSbQe2000ElibVIT_Unpack( &sZfVIT,
                                        (uint8 *)aulVITTuple,
                                        sizeof(sZfVIT) );

    /*
     * Determine what type of entry it is
     */
    if( TRUE == Entry.bOpValid )
    {
        /* VIT is a Control Record and the Op field is valid */
        SB_ASSERT( Entry.Op < SB_QE2000_ELIB_VIT_OP_MAX );
        ulNewVIR = Entry.Op;
    }
    else
    {
        /*
         * VIT is an Indirection Record and the ulPtr
         * and Cmap fields are valid
         */
        SB_ASSERT( Entry.Cmap < SB_QE2000_ELIB_VIT_CMAP_MAX );
        ulNewVIR = ( (Entry.ulPtr << 2) & 0x0000FFFC) | (Entry.Cmap);

    }

    /*
     * Set the appropriate entry in the ZFrame
     */
    switch( ulVIRNum )
    {
        case 0:
            sZfVIT.m_record0 = ulNewVIR;
            break;

        case 1:
            sZfVIT.m_record1 = ulNewVIR;
            break;

        case 2:
            sZfVIT.m_record2 = ulNewVIR;
            break;

        case 3:
            sZfVIT.m_record3 = ulNewVIR;
            break;

        /* coverity[dead_error_begin] */
        default:
            return( SB_ELIB_BAD_IDX );
            break;
    }

    (void)sbZfSbQe2000ElibVIT_Pack( &sZfVIT,
                                            (uint8 *)aulVITTuple,
                                            SB_ZF_SB_QE2000_ELIB_VIT_ENTRY_SIZE_IN_BYTES );

    nStatus = sbQe2000ElibBfMemWrite( pEp->pHalCtx,
                                      ulRecNum,
                                      aulVITTuple[0],
                                      aulVITTuple[1] );

    if( SB_ELIB_OK != nStatus )
    {
        DEXIT();
        return( nStatus );
    }

    DEXIT();
    return( SB_ELIB_OK );
}

static sbElibStatus_et sbQe2000ElibVITGet( SB_QE2000_ELIB_CONTEXT_PST pEp, uint32 ulVLAN, SB_QE2000_ELIB_VIT_PST pEntry )
{
    sbZfSbQe2000ElibVIT_t sZfVIT;
    uint32 aulVITTuple[2];
    uint32 ulRecNum;
    uint32 ulVIRNum;
    uint32 ulNewVIR;
    sbElibStatus_et nStatus;

    DENTER();
    SB_ASSERT( pEp );
    SB_ASSERT( ulVLAN < SB_QE2000_ELIB_NUM_VLANS_K );
    SB_ASSERT( pEntry );
    SB_ASSERT( pEp->pHalCtx );

    /*
     * Figure out the memory location of the VIR, and
     * the VIT tuple
     */
    ulVIRNum = ulVLAN % 4;
    ulRecNum = ulVLAN / 4;

    /*
     * Retrieve the 4-tuple entry
     */
    nStatus = sbQe2000ElibBfMemRead( pEp->pHalCtx,
                                     ulRecNum,
                                     0,
                                     &aulVITTuple[0],
                                     &aulVITTuple[1] );

    if( SB_ELIB_OK != nStatus )
    {
        DEXIT();
        return( nStatus );
    }

    sbZfSbQe2000ElibVIT_Unpack( &sZfVIT,
                                        (uint8 *)aulVITTuple,
                                        sizeof(sZfVIT) );
    /*
     * Retrive the specific entry
     */
    switch( ulVIRNum )
    {
        case 0:
            ulNewVIR = sZfVIT.m_record0;
            break;

        case 1:
            ulNewVIR = sZfVIT.m_record1;
            break;

        case 2:
            ulNewVIR = sZfVIT.m_record2;
            break;

        case 3:
            ulNewVIR = sZfVIT.m_record3;
            break;

        /* coverity[dead_error_begin] */
        default:
            return( SB_ELIB_BAD_IDX );
            break;
    }

    /*
     * Parse out the entry
     */
    if( 0xFC00 & ulNewVIR )
    {
        /*
         * We have a PTR/CMAP field pair, thus
         * a VLAN Indirection Record
         */
        pEntry->bOpValid = FALSE;
        pEntry->ulPtr = (ulNewVIR & 0x0000FFFC) >> 2;
        pEntry->Cmap = (SB_QE2000_ELIB_VIT_CMAP_T)(ulNewVIR & 0x3);
    }
    else
    {
        /*
         * We have a VLAN Control record
         */
        pEntry->Op = ulNewVIR & 0x000003FF;
        pEntry->bOpValid = TRUE;
    }

    DEXIT();
    return( SB_ELIB_OK );
}


/*
 * sbQe2000ElibVITPciSet
 *
 * Create a direct VIT entry that allows forwarding to/from
 * the PCI path.
 */
sbElibStatus_et sbQe2000ElibVITPciSet( SB_QE2000_ELIB_HANDLE Handle )
{
    SB_QE2000_ELIB_VIT_ST sVitEntry;
    sbElibStatus_et nStatus;

    SB_MEMSET(&sVitEntry, 0, sizeof(sVitEntry));
    sVitEntry.bOpValid = TRUE;
    sVitEntry.Op = SB_QE2000_ELIB_VIT_OP_FWD_NO_TAG;

    nStatus = sbQe2000ElibVITSet( Handle, 0xFFF, sVitEntry );
    if( SB_ELIB_OK != nStatus )
    {
        DEXIT();
        return( nStatus );
    }

    return( SB_ELIB_OK );
}


sbElibStatus_et sbQe2000ElibVlanFlush( SB_QE2000_ELIB_HANDLE Handle )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    uint32 ulScIdx;
    uint32 ulOffset;
    SB_QE2000_ELIB_VRT_MAP_ENTRY_PST pVrtMap;
    sbElibStatus_et nStatus;
    sbDmaMemoryHandle_t tDmaHdl;
    int i;

    DENTER();
    SB_ASSERT( Handle );
    SB_ASSERT( pEp->pHalCtx );

    /*
     * Bug 21801:  Was ScIdxMax instead of VRT Max
     */
    for( ulScIdx = 0; ulScIdx <  SBQE2000_ELIB_VRT_MAP_MAX_K; ulScIdx++ )
    {
        if( NULL == pEp->apVrtMap[ulScIdx] )
        {
            continue;
        }

        /*
         * All valid VLAN entries directly
         */
        pVrtMap = pEp->apVrtMap[ulScIdx];

        /*
         * Find our scidx within the VrtMap Entry
         */
        for( i = 0; i < SBQE2000_ELIB_SCIDX_MAX_K; i++ )
        {
            /*
             * NULL out each valid map pointer
             */
            if( SBQE2000_ELIB_SCIDX_VALID_M & pVrtMap->auwScIdx[i] )
            {
                pEp->apVrtMap[~SBQE2000_ELIB_SCIDX_VALID_M & pVrtMap->auwScIdx[i]] = NULL;
            }
        }

        /*
         * Free our internal structures
         */
        tDmaHdl.handle = NULL;
        thin_free((void *)pEp->pHalCtx,
                  SB_ALLOC_INTERNAL,
                  0, /* don't care */
                  (void*)pVrtMap->pCntrShadow,
                  tDmaHdl);
        thin_free((void *)pEp->pHalCtx,
                  SB_ALLOC_INTERNAL,
                  0, /* don't care */
                  (void*)pVrtMap,
                  tDmaHdl);

    }

    /*
     * Uninit the BF Memory Manager
     */
    nStatus = sbQe2000ElibVlanMemUninit( pEp );

    /*
     * Clear the VIT & VRT Memory
     */
    for (ulOffset = 0; ulOffset < SB_QE2000_ELIB_BF_MEM_MAX_OFFSET; ulOffset++)
    {
        nStatus = sbQe2000ElibBfMemWrite( pEp->pHalCtx, ulOffset, 0x0, 0x0 );
        if (nStatus)
        {
            DEXIT();
            return( nStatus );
        }
    }

    /*
     * Re-Init the BF Memory Manager
     */
    nStatus = sbQe2000ElibVlanMemInit( pEp );
    if( SB_ELIB_OK != nStatus )
    {
        return( nStatus );
    }

    /*
     * Re-install the PCI VIT Entry
     */
    nStatus = sbQe2000ElibVITPciSet( Handle );

    return( nStatus );
}


sbElibStatus_et sbQe2000ElibVlanRecordCreate( SB_QE2000_ELIB_HANDLE Handle, uint32 ulScIdx, uint32 ulVid,
                                  SB_QE2000_ELIB_VRT_PS_T tPortState,
                                  SB_QE2000_ELIB_VIT_CMAP_T tCmap)
{
    SB_QE2000_ELIB_VRT_MAP_ENTRY_PST pVrtMap;
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    SB_QE2000_ELIB_VIT_ST sVitEntry;
    sbDmaMemoryHandle_t tDmaHdl;
    sbZfSbQe2000ElibVRT_t zfVRT;
    uint32 aulVRT[SB_ZF_SB_QE2000_ELIB_VRT_ENTRY_SIZE/sizeof(uint32)];
    VrtPtr_t tVrtPtr;
    uint64 ullPortEnable;
    uint64 ullPopMask;
    void *pTmpVoid;
    sbElibStatus_et nStatus;
    int nNumLines;
    int nPort;
    int nNumPorts;
    int nNumCounters;
    int nPcw1Count;
    bool_t bDummyLine;

    DENTER();
    SB_ASSERT( Handle );
    SB_ASSERT( pEp->pHalCtx );
    SB_ASSERT( tCmap < SB_QE2000_ELIB_VIT_CMAP_MAX );
    SB_ASSERT( tPortState < SB_QE2000_ELIB_VRT_PS_MAX);

    if( 0xFFF == ulScIdx )
    {
        /*
         * User cannot use the PCI scidx
         */
        return( SB_ELIB_BAD_IDX );
    }

    if( SBQE2000_ELIB_VRT_MAP_MAX_K <= ulScIdx )
    {
        /*
         * scidx is outside valid range
         */
        return( SB_ELIB_BAD_IDX );
    }

    if( NULL != pEp->apVrtMap[ulScIdx] )
    {
        /*
         * This vlan is already configured
         */
        return( SB_ELIB_BAD_IDX );
    }

    if( 0xFFF < ulVid )
    {
        return( SB_ELIB_BAD_VID );
    }

    nNumPorts = 0;

    /*
     * Figure out the number of lines we'll need in the VRT memory.
     *
     * This will be defined by the number of ports, number of Class Counters
     * and the number of managed ports.
     */
    if( 1 == pEp->nOnePcwEnable )
    {
        nNumLines = 1;
    }
    else
    {
        nNumLines = 2;
    }

    if( (SB_QE2000_ELIB_VIT_CMAP_0_NO_PORT == tCmap) || (SB_QE2000_ELIB_VIT_CMAP_MAP0 == tCmap) )
    {
        nNumLines += pEp->nCMapClassCnt[0];
        nNumCounters = pEp->nCMapClassCnt[0];
    }
    else
    {
        nNumLines += pEp->nCMapClassCnt[1];
        nNumCounters = pEp->nCMapClassCnt[1];
    }

    COMPILER_64_ZERO(ullPortEnable);
    if( (SB_QE2000_ELIB_VIT_CMAP_0_NO_PORT != tCmap) && (SB_QE2000_ELIB_VIT_CMAP_1_NO_PORT != tCmap) )
    {
        /*
         * We need to add a line for each port that has a counter enabled
         */
        for( nPort = 0; nPort < pEp->nPorts; nPort++ )
        {

            if(SB_QE2000_ELIB_VRT_PS_UNMANAGED != tPortState)
            { uint64 uuTmp = COMPILER_64_INIT(0,1);
                nNumLines++;
                nNumPorts++;
                COMPILER_64_SHL(uuTmp, nPort);
                COMPILER_64_OR(ullPortEnable, uuTmp);
            }
        }

        /*
         * pEp->nPorts may not include 48 & 49
         */
        if( (1 == pEp->nOnePcwEnable) || (48 > pEp->nPorts) )
        {
            if(SB_QE2000_ELIB_VRT_PS_UNMANAGED != tPortState)
            { uint64 uuTmp = COMPILER_64_INIT(0,3);
                COMPILER_64_SHL(uuTmp, 48);
                COMPILER_64_OR(ullPortEnable, uuTmp);
                nNumLines += 2;
                nNumPorts += 2;
            }
        }
        /*
         * Add in the number of lines to clear for the port counters
         */
        nNumCounters += nNumPorts;
    }

    /*
     * NOTE:  If the number of ports that are enabled within port control
     * NOTE:  word #1 is ODD, we need to allocate an extra 'dummy' line.
     */
    COMPILER_64_SET(ullPopMask, 0, ((1 << 24) -1));
    COMPILER_64_AND(ullPopMask, ullPortEnable);
    nPcw1Count = sbQe2000ElibGetPopulationCount(ullPopMask);
    nPcw1Count += COMPILER_64_BITTEST(ullPortEnable, 48);
    nPcw1Count += COMPILER_64_BITTEST(ullPortEnable, 49);
    if(1 == (nPcw1Count % 2))
    {
        nNumLines++;
        bDummyLine = TRUE;
    }
    else
    {
        bDummyLine = FALSE;
    }

    /*
     * We now know how many lines out of the hardware Vlan Record Table
     * we need, try to 'allocate' it.  If we fail, there is no more
     * room in the VRT.
     */
    nStatus = sbQe2000ElibVlanMalloc(pEp, nNumLines, &tVrtPtr);
    if( SB_ELIB_OK != nStatus )
    {
        /*
         * No more room in the VLAN Record Table!
         */
        return( nStatus );
    }

    pTmpVoid = NULL;
    nStatus = (sbElibStatus_et) gen_thin_malloc((void *)pEp->pHalCtx,
                              SB_ALLOC_INTERNAL,
                              sizeof(*pVrtMap),
                              (void **)&pTmpVoid,
                              NULL,
                              0);

    if( SB_ELIB_OK != nStatus )
    {
        DEXIT();
        return( SB_ELIB_MEM_ALLOC_FAIL );
    }

    pVrtMap = (SB_QE2000_ELIB_VRT_MAP_ENTRY_PST)pTmpVoid;

    SB_MEMSET(pVrtMap, 0, sizeof(*pVrtMap));

    nStatus = (sbElibStatus_et) gen_thin_malloc((void *)pEp->pHalCtx,
                              SB_ALLOC_INTERNAL,
                              sizeof(SB_QE2000_ELIB_COUNTER_ST) * nNumCounters,
                              (void**)&(pVrtMap->pCntrShadow),
                              NULL,
                              0);

    if( SB_ELIB_OK != nStatus )
    {
        tDmaHdl.handle = NULL;
        thin_free((void *)pEp->pHalCtx,
                  SB_ALLOC_INTERNAL,
                  0, /* don't care */
                  pVrtMap,
                  tDmaHdl);
        return( SB_ELIB_MEM_ALLOC_FAIL );
    }

    SB_MEMSET(pVrtMap->pCntrShadow, 0, sizeof(SB_QE2000_ELIB_COUNTER_ST) * nNumCounters);

    pVrtMap->tVrtPtr = tVrtPtr;
    pVrtMap->auwScIdx[0] = (ulScIdx & SBQE2000_ELIB_SCIDX_M) | SBQE2000_ELIB_SCIDX_VALID_M;
    pVrtMap->ulNumPorts = nNumPorts;
    pVrtMap->ulVID = ulVid;
    pVrtMap->tCmap = tCmap;
    pVrtMap->nNumLines = nNumLines;
    pVrtMap->ullPortEnable = ullPortEnable;
    pVrtMap->bDummyLine = bDummyLine;

    /*
     * Fill in the VRT ZFrame
     */
    /*
     * Clean the zframe
     */
    SB_MEMSET( &zfVRT, 0, sizeof( zfVRT ) );

    /*
     * Set the port states for this entry
     * If the system uses only one port control word it is assumed that
     * all of the extraneous port states are set to unmanaged
     */
    for( nPort = 0; nPort < pEp->nPorts; nPort++ )
    {
        sbZfSbQe2000ElibVRTSetPortState( &zfVRT, nPort, tPortState );
    }

    /*
     * BUG 21822:  48 & 49 not being properly set if nPorts < 48 (ie OnePCW is enabled)
     */
    sbZfSbQe2000ElibVRTSetPortState( &zfVRT, 48, tPortState );
    sbZfSbQe2000ElibVRTSetPortState( &zfVRT, 49, tPortState );

    /*
     * Calculate the offset field if we're using two pcw's, and we are using port counts
     */
    if(0 == pEp->nOnePcwEnable)
    {
        if( (SB_QE2000_ELIB_VIT_CMAP_0_NO_PORT != tCmap) &&
            (SB_QE2000_ELIB_VIT_CMAP_1_NO_PORT != tCmap) )
        {
            /*
             * For each non-unmanaged port, add a line
             */
            if ( SB_QE2000_ELIB_VRT_PS_UNMANAGED != tPortState )
            {
                /*
                 * All PCW1 ports (0-23, 48, 49)
                 */
                zfVRT.m_nOffset = 26;
            }

            if( SB_QE2000_ELIB_VIT_CMAP_MAP0 == tCmap )
            {
                zfVRT.m_nOffset += pEp->nCMapClassCnt[0];
            }
            else
            {
                zfVRT.m_nOffset += pEp->nCMapClassCnt[1];
            }
            zfVRT.m_nOffset = zfVRT.m_nOffset << 1;
        }

        /*
         * Fill in the VID for the 2nd pcw
         */
        zfVRT.m_nVid1 = ulVid;
    }

    zfVRT.m_nVid0 = ulVid;

    sbZfSbQe2000ElibVRT_Pack( &zfVRT,
                                      (uint8 *)aulVRT,
                                      SB_ZF_SB_QE2000_ELIB_VRT_ENTRY_SIZE_IN_BYTES );

    /*
     * First clear out the VRT memory
     */
    nStatus = sbQe2000ElibVlanMemset(pEp, &tVrtPtr, 0, 0, nNumLines);
    if( SB_ELIB_OK != nStatus )
    {
        DEXIT();
        return( nStatus );
    }


    /*
     * Write out the 1st port control word
     */
    nStatus = sbQe2000ElibBfMemWrite( pEp->pHalCtx,
                                      tVrtPtr,
                                      aulVRT[1],
                                      aulVRT[0] );

    if( SB_ELIB_OK != nStatus )
    {
        DEXIT();
        return( nStatus );
    }

    tVrtPtr++;

    /*
     * If we're using two port control words, write out the second
     */
    if(0 == pEp->nOnePcwEnable)
    {
        nStatus = sbQe2000ElibBfMemWrite( pEp->pHalCtx,
                                          tVrtPtr,
                                          aulVRT[3],
                                          aulVRT[2] );

        if( SB_ELIB_OK != nStatus )
        {
            DEXIT();
            return( nStatus );
        }
        tVrtPtr++;
    }

    /*
     * VRT entry is fully written out.
     * Update the VIT entry
     */
    SB_MEMSET(&sVitEntry, 0, sizeof(sVitEntry));
    sVitEntry.bOpValid = 0;
    sVitEntry.ulPtr = pVrtMap->tVrtPtr;
    sVitEntry.Cmap = tCmap;


    nStatus = sbQe2000ElibVITSet( Handle, ulScIdx, sVitEntry );
    if( SB_ELIB_OK != nStatus )
    {
        DEXIT();
        return( nStatus );
    }

    /*
     * Save off the VRT for later use
     */
    pVrtMap->sZfVrt = zfVRT;

    /*
     * Ok, now save off our VRT Map Entry
     * and we're done.
     */
    pEp->apVrtMap[ulScIdx] = pVrtMap;

    return( SB_ELIB_OK );
}


sbElibStatus_et sbQe2000ElibVlanRecordGet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulScIdx,
                               SB_QE2000_ELIB_VIT_CMAP_PT ptCmap,
                               sbZfSbQe2000ElibVRT_t *pZf)
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;

    DENTER();
    SB_ASSERT( Handle );
    SB_ASSERT( pEp->pHalCtx );
    SB_ASSERT( pZf );
    SB_ASSERT( ptCmap );

    if( 0xFFF == ulScIdx )
    {
        /*
         * User cannot use the PCI scidx
         */
        return( SB_ELIB_BAD_IDX );
    }

    if( SBQE2000_ELIB_VRT_MAP_MAX_K <= ulScIdx )
    {
        /*
         * scidx is outside valid range
         */
        return( SB_ELIB_BAD_IDX );
    }

    if( NULL == pEp->apVrtMap[ulScIdx] )
    {
        /*
         * No record to retrieve
         */
        return( SB_ELIB_BAD_IDX );
    }
    *pZf = pEp->apVrtMap[ulScIdx]->sZfVrt;
    *ptCmap = pEp->apVrtMap[ulScIdx]->tCmap;

    return( SB_ELIB_OK );

}


sbElibStatus_et sbQe2000ElibVlanVidUpdate( SB_QE2000_ELIB_HANDLE Handle, uint32 ulScIdx, uint32 ulVid)
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    SB_QE2000_ELIB_VRT_MAP_ENTRY_PST pVrtMap;
    uint32 aulVRT[SB_ZF_SB_QE2000_ELIB_VRT_ENTRY_SIZE/sizeof(uint32)];
    uint32 ulCounts;
    VrtPtr_t tVrtPtr;
    sbElibStatus_et nStatus;

    DENTER();
    SB_ASSERT( Handle );
    SB_ASSERT( pEp->pHalCtx );

    if( 0xFFF == ulScIdx )
    {
        /*
         * User cannot use the PCI scidx
         */
        return( SB_ELIB_BAD_IDX );
    }

    if( SBQE2000_ELIB_VRT_MAP_MAX_K <= ulScIdx )
    {
        /*
         * scidx is outside valid range
         */
        return( SB_ELIB_BAD_IDX );
    }

    if( NULL == pEp->apVrtMap[ulScIdx] )
    {
        /*
         * No record to change
         */
        return( SB_ELIB_BAD_IDX );
    }

    if( 0xFFF < ulVid )
    {
        return( SB_ELIB_BAD_VID );
    }

    pVrtMap = pEp->apVrtMap[ulScIdx];
    tVrtPtr = pVrtMap->tVrtPtr;

    /*
     * Accumulate the counters associated with this ScIdx
     */
    ulCounts = 0;
    nStatus = sbQe2000ElibScIdxCountAccumulate( pEp, ulScIdx, &ulCounts );
    if( SB_ELIB_OK != nStatus )
    {
        return( nStatus );
    }


    /*
     * Update the VID
     */
    pVrtMap->ulVID = ulVid;
    pVrtMap->sZfVrt.m_nVid1 = ulVid;
    pVrtMap->sZfVrt.m_nVid0 = ulVid;
    sbZfSbQe2000ElibVRT_Pack( &pVrtMap->sZfVrt,
                                      (uint8 *)aulVRT,
                                      SB_ZF_SB_QE2000_ELIB_VRT_ENTRY_SIZE_IN_BYTES );

    /*
     * Write out the 1st port control word
     */
    nStatus = sbQe2000ElibBfMemWrite( pEp->pHalCtx,
                                      tVrtPtr,
                                      aulVRT[1],
                                      aulVRT[0] );

    if( SB_ELIB_OK != nStatus )
    {
        DEXIT();
        return( nStatus );
    }

    /*
     * If we're using two port control words, write out the second
     */
    if(0 == pEp->nOnePcwEnable)
    {
        nStatus = sbQe2000ElibBfMemWrite( pEp->pHalCtx,
                                          tVrtPtr + 1,
                                          aulVRT[3],
                                          aulVRT[2] );

        if( SB_ELIB_OK != nStatus )
        {
            DEXIT();
            return( nStatus );
        }
    }

    return( SB_ELIB_OK );
}

sbElibStatus_et sbQe2000ElibVlanPortStateSet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulScIdx, uint32 ulPort,
                                  SB_QE2000_ELIB_VRT_PS_T tPortState )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    SB_QE2000_ELIB_VRT_MAP_ENTRY_PST pVrtMap;
    SB_QE2000_ELIB_COUNTER_PST pTmpCounterShadow = NULL;
    SB_QE2000_ELIB_VRT_PS_T tOrigPortState;
    SB_QE2000_ELIB_VIT_ST sVitEntry;
    void *pTmpVoid;
    sbDmaMemoryHandle_t tDmaHdl;
    uint64 ullPopMask;
    uint32 aulVRT[SB_ZF_SB_QE2000_ELIB_VRT_ENTRY_SIZE/sizeof(uint32)];
    VrtPtr_t tVrtPtr, tNewVrtPtr;
    int nClassCnts;
    int nCountIdx;
    int nPortIdx;
    uint32 ulCounts;
    sbElibStatus_et nStatus;
    int nLines;
    int nPort;
    int nAdd;
    int i;

    DENTER();
    SB_ASSERT( Handle );
    SB_ASSERT( pEp->pHalCtx );

    if( 0xFFF == ulScIdx )
    {
        /*
         * User cannot use the PCI scidx
         */
        return( SB_ELIB_BAD_IDX );
    }

    if( SBQE2000_ELIB_VRT_MAP_MAX_K <= ulScIdx )
    {
        /*
         * scidx is outside valid range
         */
        return( SB_ELIB_BAD_IDX );
    }

    if( NULL == pEp->apVrtMap[ulScIdx] )
    {
        /*
         * No record to change
         */
        return( SB_ELIB_BAD_IDX );
    }

    pVrtMap = pEp->apVrtMap[ulScIdx];
    tVrtPtr = pVrtMap->tVrtPtr;
    tNewVrtPtr = tVrtPtr;

    tOrigPortState = sbZfSbQe2000ElibVRTGetPortState(&pVrtMap->sZfVrt, ulPort);

    if(tPortState == tOrigPortState)
    {
        return( SB_ELIB_OK );
    }

    /*
     * Accumulate the counters associated with this ScIdx
     */
    ulCounts = 0;
    nStatus = sbQe2000ElibScIdxCountAccumulate( pEp, ulScIdx, &ulCounts );
    if( SB_ELIB_OK != nStatus )
    {
        return( nStatus );
    }

    nAdd = -1;

    if( (SB_QE2000_ELIB_VRT_PS_UNMANAGED == tOrigPortState) &&
        (SB_QE2000_ELIB_VRT_PS_UNMANAGED != tPortState) )
    { uint64 uuTmp = COMPILER_64_INIT(0,1);
        /*
         * The port is coming on line for 'management'
         */
        COMPILER_64_SHL(uuTmp, ulPort);
        COMPILER_64_OR(pVrtMap->ullPortEnable, uuTmp);
        pVrtMap->ulNumPorts++;
        if( (SB_QE2000_ELIB_VIT_CMAP_0_NO_PORT != pVrtMap->tCmap) &&
            (SB_QE2000_ELIB_VIT_CMAP_1_NO_PORT != pVrtMap->tCmap) )
        {
            pVrtMap->nNumLines++;
        }

        /* Allocate a new counter shadow */
        if(0 == pEp->nOnePcwEnable)
        {
            nLines = pVrtMap->nNumLines - 2;
        }
        else
        {
            nLines = pVrtMap->nNumLines - 1;
        }
        nAdd = 1;

        if( (SB_QE2000_ELIB_VIT_CMAP_0_NO_PORT != pVrtMap->tCmap) &&
            (SB_QE2000_ELIB_VIT_CMAP_1_NO_PORT != pVrtMap->tCmap) )
        {
            if( (23 >= ulPort) || (48 == ulPort) || (49 == ulPort) )
            {
                if(TRUE == pVrtMap->bDummyLine)
                {
                    /*
                     * We had an odd number of ports, now we don't
                     */
                    pVrtMap->bDummyLine = FALSE;
                    pVrtMap->nNumLines--;
                }
                else
                {
                    /*
                     * We now have an odd number of ports in the 1st pcw
                     */
                    pVrtMap->bDummyLine = TRUE;
                    pVrtMap->nNumLines++;
                }
            }
        }
    }
    else if( (SB_QE2000_ELIB_VRT_PS_UNMANAGED != tOrigPortState) &&
             (SB_QE2000_ELIB_VRT_PS_UNMANAGED == tPortState)  )
    { uint64 uuTmp = COMPILER_64_INIT(0,1);
        /* Port is going from Managed to unmanaged */
        COMPILER_64_SHL(uuTmp, ulPort);
        COMPILER_64_NOT(uuTmp);
        COMPILER_64_AND(pVrtMap->ullPortEnable, uuTmp);
        pVrtMap->ulNumPorts--;
        if( (SB_QE2000_ELIB_VIT_CMAP_0_NO_PORT != pVrtMap->tCmap) &&
            (SB_QE2000_ELIB_VIT_CMAP_1_NO_PORT != pVrtMap->tCmap) )
        {
            pVrtMap->nNumLines--;
        }

        /* Allocate a new counter shadow */
        if(0 == pEp->nOnePcwEnable)
        {
            nLines = pVrtMap->nNumLines - 2;
        }
        else
        {
            nLines = pVrtMap->nNumLines - 1;
        }
        nAdd = 0;

        if( (SB_QE2000_ELIB_VIT_CMAP_0_NO_PORT != pVrtMap->tCmap) &&
            (SB_QE2000_ELIB_VIT_CMAP_1_NO_PORT != pVrtMap->tCmap) )
        {
            if( (23 >= ulPort) || (48 == ulPort) || (49 == ulPort) )
            {
                if(TRUE == pVrtMap->bDummyLine)
                {
                    /*
                     * We had an odd number of ports, now we don't
                     */
                    pVrtMap->bDummyLine = FALSE;
                    pVrtMap->nNumLines--;
                }
                else
                {
                    /*
                     * We now have an odd number of ports in the 1st pcw
                     */
                    pVrtMap->bDummyLine = TRUE;
                    pVrtMap->nNumLines++;
                }
            }
        }
    }
    else
    {
        /*
         * Port state does not require adding or removing port counts
         */
        nLines = 0;
    }

    /*
     * Set the new port state in the PCW
     */
    sbZfSbQe2000ElibVRTSetPortState(&pVrtMap->sZfVrt, ulPort, tPortState);

    if( -1 != nAdd)
    {
        /*
         * Attempt to re-allocate our VRT entry
         */
        nStatus = sbQe2000ElibVlanRealloc(pEp, pVrtMap->nNumLines, &tNewVrtPtr);
        if( SB_ELIB_OK != nStatus )
        {
            /*
             * If realloc fails, the VRT may no longer be valid.  We have two cases
             * here, if the free failed, the VRT is valid, as nothing has been
             * done to the VRT memory.  However, the block may not have been able
             * to be found.  The user must re-instantiate the VLAN.
             */
            if( SB_ELIB_VLAN_MEM_FREE_FAIL == nStatus )
            {
                /*
                 * We have an orphaned block. If the free failed we
                 * were unable to find the DLL entry.  There is no
                 * going back. We need to manually remove our shadow info.
                 */
            }
            else if( SB_ELIB_VLAN_MEM_ALLOC_FAIL == nStatus )
            {
                /*
                 * We're out of BF memory.  We've already freed
                 * the memory, but we still don't have enough
                 * contiguous lines for our VRT entry.
                 * We need to manually remove our shadow info
                 */
            }
            for( i = 0; i < SBQE2000_ELIB_SCIDX_MAX_K; i++ )
            {
                /*
                 * NULL out each valid map pointer
                 */
                if( SBQE2000_ELIB_SCIDX_VALID_M & pVrtMap->auwScIdx[i] )
                {
                    sbQe2000ElibVlanScIdxDetach(Handle,
                                                ~SBQE2000_ELIB_SCIDX_VALID_M & pVrtMap->auwScIdx[i]);
                }
            }
            tDmaHdl.handle = NULL;
            thin_free((void *)pEp->pHalCtx,
                      SB_ALLOC_INTERNAL,
                      0, /* don't care */
                      (void*)pVrtMap->pCntrShadow,
                      tDmaHdl);
            thin_free((void *)pEp->pHalCtx,
                      SB_ALLOC_INTERNAL,
                      0, /* don't care */
                      (void*)pVrtMap,
                      tDmaHdl);

            return( nStatus );
        }

        nStatus = sbQe2000ElibVlanMemset(pEp, &tNewVrtPtr, 0, 0, pVrtMap->nNumLines);
        if( SB_ELIB_OK != nStatus )
        {
            return( nStatus );
        }

        if((SB_QE2000_ELIB_VIT_CMAP_0_NO_PORT != pVrtMap->tCmap) &&
           (SB_QE2000_ELIB_VIT_CMAP_1_NO_PORT != pVrtMap->tCmap) )
        {

            /*
             * We need not allocate the dummy line for the shadow counters
             */
            pTmpCounterShadow = NULL;
            pTmpVoid = (void*)pTmpCounterShadow;
            nStatus = (sbElibStatus_et) gen_thin_malloc((void *)pEp->pHalCtx,
                                      SB_ALLOC_INTERNAL,
                                      sizeof(SB_QE2000_ELIB_COUNTER_ST) * nLines,
                                      (void **)&pTmpVoid,
                                      NULL,
                                      0);

            if( SB_ELIB_OK != nStatus )
            {
                DEXIT();
                return( SB_ELIB_MEM_ALLOC_FAIL );
            }

            pTmpCounterShadow = (SB_QE2000_ELIB_COUNTER_PST)pTmpVoid;

            SB_MEMSET(pTmpCounterShadow, 0, sizeof(SB_QE2000_ELIB_COUNTER_ST) * nLines);

            /*
             * We can't just SB_MEMCPY the counts, as the port is possibly in the
             * middle of our count structure
             */
            if(SB_QE2000_ELIB_VIT_CMAP_MAP0 == pVrtMap->tCmap)
            {
                nClassCnts = pEp->nCMapClassCnt[0];
            }
            else
            {
                nClassCnts = pEp->nCMapClassCnt[1];
            }

            for(i = 0; i < nClassCnts; i++)
            {
                pTmpCounterShadow[i].ullPktCount = pVrtMap->pCntrShadow[i].ullPktCount;
                pTmpCounterShadow[i].ullByteCount = pVrtMap->pCntrShadow[i].ullByteCount;
            }

            if( (SB_QE2000_ELIB_VIT_CMAP_0_NO_PORT != pVrtMap->tCmap) &&
                (SB_QE2000_ELIB_VIT_CMAP_1_NO_PORT != pVrtMap->tCmap) )
            {
                /*
                 * Now we need to get the port counts straightened out
                 *
                 * Algorithm for managing the counter shadow
                 *
                 * shadow all counts below the port change;
                 *
                 * if we're adding a port, zero that ports counter;
                 * if we're removing a port, skip the original count;
                 *
                 * shadow all counts above the port change;
                 */
                nPort = 0;
                nCountIdx = nClassCnts;

                COMPILER_64_SET(ullPopMask, 0, 1);
                COMPILER_64_SHL(ullPopMask, ulPort);
                COMPILER_64_SUB_32(ullPopMask, 1);
                COMPILER_64_AND(ullPopMask, pVrtMap->ullPortEnable);
                nPort = sbQe2000ElibGetPopulationCount(ullPopMask);
                for( i = nCountIdx; i < nPort + nClassCnts; i++ )
                {
                    pTmpCounterShadow[nCountIdx].ullPktCount = pVrtMap->pCntrShadow[i].ullPktCount;
                    pTmpCounterShadow[nCountIdx].ullByteCount = pVrtMap->pCntrShadow[i].ullByteCount;
                    nCountIdx++;
                }

                if( 1 == nAdd )
                {
                    COMPILER_64_ZERO(pTmpCounterShadow[nCountIdx].ullPktCount);
                    COMPILER_64_ZERO(pTmpCounterShadow[nCountIdx].ullByteCount);
                    nCountIdx++;
                }
                else
                {
                    /*
                     * We're removing a port, just skip it
                     */
                }

                COMPILER_64_SET(ullPopMask, 0, 1);
                COMPILER_64_SHL(ullPopMask, (ulPort + 1));
                COMPILER_64_SUB_32(ullPopMask, 1);
                COMPILER_64_NOT(ullPopMask);
                { uint64 uuTmp = COMPILER_64_INIT(0,1);
                  COMPILER_64_SHL(uuTmp, ulPort);
                  COMPILER_64_AND(ullPopMask, uuTmp);
                }
                nPortIdx = nPort + nClassCnts;
                COMPILER_64_AND(ullPopMask, pVrtMap->ullPortEnable);
                nPort = sbQe2000ElibGetPopulationCount(ullPopMask);
                for( i = nCountIdx; i < nPort + nPortIdx; i++ )
                {
                    pTmpCounterShadow[nCountIdx].ullPktCount = pVrtMap->pCntrShadow[i].ullPktCount;
                    pTmpCounterShadow[nCountIdx].ullByteCount = pVrtMap->pCntrShadow[i].ullByteCount;
                    nCountIdx++;
                }
            }

            tDmaHdl.handle = NULL;
            thin_free((void *)pEp->pHalCtx,
                      SB_ALLOC_INTERNAL,
                      0, /* don't care */
                      (void*)pVrtMap->pCntrShadow,
                      tDmaHdl);
            pVrtMap->pCntrShadow = pTmpCounterShadow;
        }
    }


    /*
     * All the counter management is done, we need to pack up our zframe
     * and jam it into the memory.
     * Calculate the offset field if we're using two pcw's,
     * and we are using port counts
     */
    if(0 == pEp->nOnePcwEnable)
    {
        if( (SB_QE2000_ELIB_VIT_CMAP_0_NO_PORT != pVrtMap->tCmap) &&
            (SB_QE2000_ELIB_VIT_CMAP_1_NO_PORT != pVrtMap->tCmap) )
        {
            /*
             * For each non-unmanaged port, add a line
             */
            pVrtMap->sZfVrt.m_nOffset = 0;
            for(nPort = 0; nPort < 24; nPort++)
            {
                if( SB_QE2000_ELIB_VRT_PS_UNMANAGED !=
                    sbZfSbQe2000ElibVRTGetPortState(&pVrtMap->sZfVrt, nPort))
                {
                    pVrtMap->sZfVrt.m_nOffset++;
                }
            }

            if( SB_QE2000_ELIB_VRT_PS_UNMANAGED !=
                sbZfSbQe2000ElibVRTGetPortState(&pVrtMap->sZfVrt, 48) )
            {
                pVrtMap->sZfVrt.m_nOffset++;
            }
            if( SB_QE2000_ELIB_VRT_PS_UNMANAGED !=
                sbZfSbQe2000ElibVRTGetPortState(&pVrtMap->sZfVrt, 49) )
            {
                pVrtMap->sZfVrt.m_nOffset++;
            }

            if( SB_QE2000_ELIB_VIT_CMAP_MAP0 == pVrtMap->tCmap )
            {
                pVrtMap->sZfVrt.m_nOffset += pEp->nCMapClassCnt[0];
            }
            else
            {
                pVrtMap->sZfVrt.m_nOffset += pEp->nCMapClassCnt[1];
            }
            pVrtMap->sZfVrt.m_nOffset = pVrtMap->sZfVrt.m_nOffset << 1;
        }

    }

    sbZfSbQe2000ElibVRT_Pack(&pVrtMap->sZfVrt, (uint8*)aulVRT, 0);


    if( -1 == nAdd )
    {
        if((SB_QE2000_ELIB_VIT_CMAP_0_NO_PORT != pVrtMap->tCmap) &&
           (SB_QE2000_ELIB_VIT_CMAP_1_NO_PORT != pVrtMap->tCmap) )
        {
            /*
             * ENHANCEME:  We need to transition the port
             */
            /*
             * Transition the port from disable to enable or
             * vice versa.  for now, just reset the count to zero
             */
            /* Address DEFECT 22280 */

            COMPILER_64_SET(ullPopMask, 0, 1);
            COMPILER_64_SHL(ullPopMask, ulPort);
            COMPILER_64_SUB_32(ullPopMask, 1);
            COMPILER_64_AND(ullPopMask, pVrtMap->ullPortEnable);
            nPort = sbQe2000ElibGetPopulationCount(ullPopMask);
            if( SB_QE2000_ELIB_VIT_CMAP_MAP0 == pVrtMap->tCmap )
            {
                nPort += pEp->nCMapClassCnt[0];
            }
            else
            {
                nPort += pEp->nCMapClassCnt[1];
            }

            COMPILER_64_ZERO(pVrtMap->pCntrShadow[nPort].ullPktCount);
            COMPILER_64_ZERO(pVrtMap->pCntrShadow[nPort].ullByteCount);
        }
    }

    /*
     * Write out the 1st port control word
     */
    nStatus = sbQe2000ElibBfMemWrite( pEp->pHalCtx,
                                      tNewVrtPtr,
                                      aulVRT[1],
                                      aulVRT[0] );

    if( SB_ELIB_OK != nStatus )
    {
        DEXIT();
        return( nStatus );
    }

    /*
     * If we're using two port control words, write out the second
     */
    if(0 == pEp->nOnePcwEnable)
    {
        nStatus = sbQe2000ElibBfMemWrite( pEp->pHalCtx,
                                          tNewVrtPtr + 1,
                                          aulVRT[3],
                                          aulVRT[2] );

        if( SB_ELIB_OK != nStatus )
        {
            DEXIT();
            return( nStatus );
        }
    }

    /*
     * Update our VRT Map Entry with the (possibly) new vrt pointer
     */
    pVrtMap->tVrtPtr = tNewVrtPtr;

    /*
     * Now update all VIT entries to the new VRT Ptr.
     */
    for( i = 0; i < SBQE2000_ELIB_SCIDX_MAX_K; i++ )
    {
        /* Find the first empty scidx entry */
        if( (SBQE2000_ELIB_SCIDX_VALID_M & pVrtMap->auwScIdx[i]) )
        {
            /*
             * We have a place for our new scidx
             * Grab the VIT entry for the original scidx and copy it
             * to the new VIT.
             */
            ulScIdx = ~SBQE2000_ELIB_SCIDX_VALID_M & (pVrtMap->auwScIdx[i]);
            nStatus = sbQe2000ElibVITGet( pEp, ulScIdx, &sVitEntry );
            if( SB_ELIB_OK != nStatus )
            {
                return( nStatus );
            }

            sVitEntry.ulPtr = tNewVrtPtr;
            nStatus = sbQe2000ElibVITSet( pEp, ulScIdx, sVitEntry );
            if( SB_ELIB_OK != nStatus )
            {
                return( nStatus );
            }
        }
    }

    return( SB_ELIB_OK );
}

sbElibStatus_et sbQe2000ElibVlanPortStateGet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulScIdx,
                                  uint32 ulPort, SB_QE2000_ELIB_VRT_PS_T *ptPortState)
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    SB_QE2000_ELIB_VRT_MAP_ENTRY_PST pVrtMap;

    DENTER();
    SB_ASSERT( Handle );
    SB_ASSERT( pEp->pHalCtx );
    SB_ASSERT( ptPortState );

    if( 0xFFF == ulScIdx )
    {
        /*
         * User cannot use the PCI scidx
         */
        return( SB_ELIB_BAD_IDX );
    }

    if( SBQE2000_ELIB_VRT_MAP_MAX_K <= ulScIdx )
    {
        /*
         * scidx is outside valid range
         */
        return( SB_ELIB_BAD_IDX );
    }

    if( NULL == pEp->apVrtMap[ulScIdx] )
    {
        /*
         * No record to retrieve info from
         */
        return( SB_ELIB_BAD_IDX );
    }

    pVrtMap = pEp->apVrtMap[ulScIdx];
    /*
     * Send back the cached copy
     */
    *ptPortState = sbZfSbQe2000ElibVRTGetPortState(&pVrtMap->sZfVrt, ulPort);

    return( SB_ELIB_OK );
}

sbElibStatus_et sbQe2000ElibVlanScIdxAttach( SB_QE2000_ELIB_HANDLE Handle, uint32 ulScIdx,
                                       uint32 ulScIdxAttach )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    SB_QE2000_ELIB_VRT_MAP_ENTRY_PST pVrtMap;
    SB_QE2000_ELIB_VIT_ST sVitEntry;
    int nAttachIdx;
    sbElibStatus_et nStatus;
    int i;

    DENTER();
    SB_ASSERT( Handle );
    SB_ASSERT( pEp->pHalCtx );
    SB_ASSERT( ulScIdxAttach < SBQE2000_ELIB_VRT_MAP_MAX_K );

    if( 0xFFF == ulScIdx )
    {
        /*
         * User cannot use the PCI scidx
         */
        return( SB_ELIB_BAD_IDX );
    }

    if( SBQE2000_ELIB_VRT_MAP_MAX_K <= ulScIdx )
    {
        /*
         * scidx is outside valid range
         */
        return( SB_ELIB_BAD_IDX );
    }

    if( NULL == pEp->apVrtMap[ulScIdx] )
    {
        /*
         * No record to attach to
         */
        return( SB_ELIB_BAD_IDX );
    }

    pVrtMap = pEp->apVrtMap[ulScIdx];

    /*
     * Ensure we have enough room for the new scidx.  The first scidx may have
     * been removed, so we scan all of them.
     */
    nAttachIdx = -1;
    for( i = 0; i < SBQE2000_ELIB_SCIDX_MAX_K; i++ )
    {
        /* Find the first empty scidx entry */
        if( !(SBQE2000_ELIB_SCIDX_VALID_M & pVrtMap->auwScIdx[i]) )
        {
            nAttachIdx = i;
            break;
        }
    }

    if( -1 == nAttachIdx )
    {
        /*
         * No more room to attach a new scidx
         */
        return( SB_ELIB_BAD_IDX );
    }

    /*
     * We have a place for our new scidx
     * Grab the VIT entry for the original scidx and copy it
     * to the new VIT.
     */
    nStatus = sbQe2000ElibVITGet( pEp, ulScIdx, &sVitEntry );
    if( SB_ELIB_OK != nStatus )
    {
        return( nStatus );
    }

    nStatus = sbQe2000ElibVITSet( pEp, ulScIdxAttach, sVitEntry );

    pVrtMap->auwScIdx[nAttachIdx] = (ulScIdxAttach & SBQE2000_ELIB_SCIDX_M) | SBQE2000_ELIB_SCIDX_VALID_M;
    pEp->apVrtMap[ulScIdxAttach] = pVrtMap;

    return( SB_ELIB_OK );
}

sbElibStatus_et sbQe2000ElibVlanScIdxDetach( SB_QE2000_ELIB_HANDLE Handle, uint32 ulScIdx )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    SB_QE2000_ELIB_VRT_MAP_ENTRY_PST pVrtMap;
    SB_QE2000_ELIB_VIT_ST sVitEntry;
    int nDetachIdx;
    sbElibStatus_et nStatus;
    int i;

    DENTER();
    SB_ASSERT( Handle );
    SB_ASSERT( pEp->pHalCtx );
    SB_ASSERT( ulScIdx < SBQE2000_ELIB_VRT_MAP_MAX_K );

    if( 0xFFF == ulScIdx )
    {
        /*
         * User cannot use the PCI scidx
         */
        return( SB_ELIB_BAD_IDX );
    }

    if( SBQE2000_ELIB_VRT_MAP_MAX_K <= ulScIdx )
    {
        /*
         * scidx is outside valid range
         */
        return( SB_ELIB_BAD_IDX );
    }

    if( NULL == pEp->apVrtMap[ulScIdx] )
    {
        /*
         * No record to detach from
         */
        return( SB_ELIB_BAD_IDX );
    }

    pVrtMap = pEp->apVrtMap[ulScIdx];

    /*
     * Find our scidx within the VrtMap Entry
     */
    nDetachIdx = -1;
    for( i = 0; i < SBQE2000_ELIB_SCIDX_MAX_K; i++ )
    {
        if( (SBQE2000_ELIB_SCIDX_VALID_M & pVrtMap->auwScIdx[i]) )
        {
            if( (~SBQE2000_ELIB_SCIDX_VALID_M & pVrtMap->auwScIdx[i]) == ulScIdx)
            {
                nDetachIdx = i;
            }
            break;
        }
    }

    if( -1 == nDetachIdx )
    {
        /*
         * We have a SERIOUS problem.  Somehow we've indexed
         * into a VRT Map entry that doesn't match our scidx.
         */
        return( SB_ELIB_BAD_IDX );
    }

    /*
     * Reset the scidx entry in our VRT Map Entry
     */
    pVrtMap->auwScIdx[nDetachIdx] = 0;

    /*
     * Set the VIT entry at the scidx to drop, no notify
     */
    SB_MEMSET(&sVitEntry, 0, sizeof(sVitEntry));
    sVitEntry.bOpValid = TRUE;
    sVitEntry.Op = SB_QE2000_ELIB_VIT_OP_DROP_NO_NOTIFY;

    nStatus =  sbQe2000ElibVITSet( pEp, ulScIdx, sVitEntry );
    if( SB_ELIB_OK != nStatus )
    {
        return( nStatus );
    }

    return( SB_ELIB_OK );
}

sbElibStatus_et sbQe2000ElibVlanRecordDelete( SB_QE2000_ELIB_HANDLE Handle, uint32 ulScIdx )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    SB_QE2000_ELIB_VRT_MAP_ENTRY_PST pVrtMap;
    SB_QE2000_ELIB_VIT_ST sVitEntry;
    sbDmaMemoryHandle_t tDmaHdl;
    uint32 aulDetachScIdx[SBQE2000_ELIB_SCIDX_MAX_K];
    int nDetachIdx;
    sbElibStatus_et nStatus;
    int i;

    DENTER();
    SB_ASSERT( Handle );
    SB_ASSERT( pEp->pHalCtx );
    SB_ASSERT( ulScIdx < SBQE2000_ELIB_VRT_MAP_MAX_K );

    if( 0xFFF == ulScIdx )
    {
        /*
         * User cannot use the PCI scidx
         */
        return( SB_ELIB_BAD_IDX );
    }

    if( SBQE2000_ELIB_VRT_MAP_MAX_K <= ulScIdx )
    {
        /*
         * scidx is outside valid range
         */
        return( SB_ELIB_BAD_IDX );
    }

    if( NULL == pEp->apVrtMap[ulScIdx] )
    {
        /*
         * No record to detach from
         */
        return( SB_ELIB_BAD_IDX );
    }

    pVrtMap = pEp->apVrtMap[ulScIdx];

    /*
     * Find our scidx within the VrtMap Entry
     */
    for( i = 0; i < SBQE2000_ELIB_SCIDX_MAX_K; i++)
    {
        aulDetachScIdx[i] = 0;
    }

    nDetachIdx = 0;
    for( i = 0; i < SBQE2000_ELIB_SCIDX_MAX_K; i++ )
    {
        /*
         * Retain each valid ScIdx
         */
        if( SBQE2000_ELIB_SCIDX_VALID_M & pVrtMap->auwScIdx[i] )
        {
            aulDetachScIdx[nDetachIdx] = ~SBQE2000_ELIB_SCIDX_VALID_M & pVrtMap->auwScIdx[i];
            nDetachIdx++;
            /*
             * Clear out the map
             */
            pVrtMap->auwScIdx[i] = 0;
        }
    }

    SB_MEMSET(&sVitEntry, 0, sizeof(sVitEntry));
    for( i = 0; i < nDetachIdx; i++ )
    {
        /*
         * For each ScIdx we need to do the following:
         *    NULL out the VrtMap Entry
         *    Set the VIT for the ScIdx to drop no notify
         */

        sVitEntry.bOpValid = TRUE;
        sVitEntry.Op = SB_QE2000_ELIB_VIT_OP_DROP_NOTIFY;

        nStatus =  sbQe2000ElibVITSet( pEp, aulDetachScIdx[i], sVitEntry );
        if( SB_ELIB_OK != nStatus )
        {
            return( nStatus );
        }
        pEp->apVrtMap[aulDetachScIdx[i]] = NULL;
    }

    /*
     * Now that all of the VITs are gone, we can remove our VRT entry
     */
    nStatus = sbQe2000ElibVlanFree(pEp, pVrtMap->nNumLines, &(pVrtMap->tVrtPtr));
    if( SB_ELIB_OK != nStatus )
    {
        /*
         * This is really bad.
         */
        return( nStatus );
    }

    /*
     * Free our internal structures
     */
    tDmaHdl.handle = NULL;
    thin_free((void *)pEp->pHalCtx,
              SB_ALLOC_INTERNAL,
              0, /* don't care */
              (void*)pVrtMap->pCntrShadow,
              tDmaHdl);
    thin_free((void *)pEp->pHalCtx,
              SB_ALLOC_INTERNAL,
              0, /* don't care */
              (void*)pVrtMap,
              tDmaHdl);

    return( SB_ELIB_OK );
}


sbElibStatus_et sbQe2000ElibVlanCountAccumulateGet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulScIdx,
                                        bool_t bClrOnRd,
                                        SB_QE2000_ELIB_COUNTER_ST aCounts[SB_QE2000_ELIB_MAX_COUNTERS_K])
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    sbElibStatus_et nStatus;
    uint32 ulCounts;

    SB_ASSERT( Handle );
    SB_ASSERT( pEp->pHalCtx );
    if (ulScIdx >= SBQE2000_ELIB_VRT_MAP_MAX_K) {
        return ( SB_ELIB_BAD_ARGS);
    }

    ulCounts = 0;
    nStatus = sbQe2000ElibScIdxCountAccumulate( pEp, ulScIdx, &ulCounts );
    /* Address DEFECT 22280, not checking status properly */
    if( SB_ELIB_OK != nStatus )
    {
        return( nStatus );
    }

    nStatus = sbQe2000ElibVlanCountGetAll( Handle, ulScIdx, bClrOnRd, aCounts );

    if( SB_ELIB_OK != nStatus )
    {
        return( nStatus );
    }

    return( SB_ELIB_OK );
}

sbElibStatus_et sbQe2000ElibVlanCountGet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulScIdx,
                              SB_QE2000_ELIB_VLAN_COUNT_INDEX_T tCntIdx,
                              bool_t bClrOnRd,
                              SB_QE2000_ELIB_COUNTER_PST pCounts)
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    SB_QE2000_ELIB_VRT_MAP_ENTRY_PST pVrtMap;
    uint64 ullPopMask;
#if 0
    int nCounts;
#endif
    int nCountFwdIdx;
    int nPortIdx;
    int nCmap;

    SB_ASSERT( Handle );
    SB_ASSERT( pEp->pHalCtx );
    if (ulScIdx >= SBQE2000_ELIB_VRT_MAP_MAX_K) {
        return (SB_ELIB_BAD_ARGS);
    }
    SB_ASSERT( pCounts );

    COMPILER_64_ZERO(pCounts->ullPktCount);
    COMPILER_64_ZERO(pCounts->ullByteCount);

    pVrtMap = pEp->apVrtMap[ulScIdx];

    if( NULL == pVrtMap )
    {
        return( SB_ELIB_BAD_IDX );
    }

#if 0
    /*
     * Account for the Port Control Word
     */
    if( 0 == pEp->nOnePcwEnable )
    {
        nCounts = pVrtMap->nNumLines - 2;
    }
    else
    {
        nCounts = pVrtMap->nNumLines - 1;
    }
#endif

    /*
     * Figure out the class mapping
     */
    if( (SB_QE2000_ELIB_VIT_CMAP_0_NO_PORT == pVrtMap->tCmap) ||
            (SB_QE2000_ELIB_VIT_CMAP_MAP0 == pVrtMap->tCmap) )
    {
        nCmap = 0;
    }
    else
    {
        nCmap = 1;
    }


    if( (SB_QE2000_ELIB_CNT_CLASS0 <= tCntIdx) && (SB_QE2000_ELIB_CNT_CLASS15 >= tCntIdx) )
    {
        /* tbs - 4 Apr 2006, Address DEFECT 22642 */
        if( (pEp->nCMapClassCnt[nCmap] >= tCntIdx) && (tCntIdx < pEp->nCMapClassCnt[nCmap]) )
        {
            pCounts->ullPktCount  = pVrtMap->pCntrShadow[tCntIdx].ullPktCount;
            pCounts->ullByteCount = pVrtMap->pCntrShadow[tCntIdx].ullByteCount;
            if(TRUE == bClrOnRd)
            {
                COMPILER_64_ZERO(pVrtMap->pCntrShadow[tCntIdx].ullPktCount);
                COMPILER_64_ZERO(pVrtMap->pCntrShadow[tCntIdx].ullByteCount);
            }
        }
        else
        {
            /*
             * User requested a count that does not exist
             */
            return( SB_ELIB_BAD_IDX );
        }
        return( SB_ELIB_OK );
    }

    /*
     * Otherwise, we're trying to get a Port Count.
     *
     * We need to figure out the first port count index.  Then, if the port is in some
     * sort of managed state, we can give them the counts.
     */
    if( (SB_QE2000_ELIB_VIT_CMAP_0_NO_PORT == pVrtMap->tCmap) ||
        (SB_QE2000_ELIB_VIT_CMAP_1_NO_PORT == pVrtMap->tCmap) )
    {
        /*
         * There are NO port counters...
         */
        return( SB_ELIB_BAD_IDX );
    }

    /*
     * Ensure the port is in some managed state
     */
    nPortIdx = tCntIdx - SB_QE2000_ELIB_CNT_PORT0;
    if( !COMPILER_64_BITTEST(pVrtMap->ullPortEnable, nPortIdx) )
    {
        /*
         * Port is in unmanaged state, there are no counters
         * for this port
         */
        return( SB_ELIB_BAD_IDX );
    }

    /*
     * Due to the ordering of the PCW, we need to manupulate
     * the enable to figure out where the count is.
     */
    if(24 > nPortIdx)
    {
        COMPILER_64_SET(ullPopMask, 0, ((1 << nPortIdx) - 1)); 
    }
    else if(48 == nPortIdx)
    {
        COMPILER_64_SET(ullPopMask, 1 << 16, ((1 << 24) - 1));
    }
    else if(49 == nPortIdx)
    {
        COMPILER_64_SET(ullPopMask, 1 << 17, ((1 << 24) - 1));
    }
    else
    {
        if (nPortIdx < 32) {
            COMPILER_64_SET(ullPopMask, 3 << 16, ((1 << nPortIdx) - 1));
        } else {
            COMPILER_64_SET(ullPopMask, (3 << 16) | ((1 << (nPortIdx - 32)) - 1), 0xFFFFFFFF);
        }
    }

    /*
     * Get the actual index into the shadow table
     */
    COMPILER_64_AND(ullPopMask, pVrtMap->ullPortEnable);
    nPortIdx = sbQe2000ElibGetPopulationCount(ullPopMask);

    /*
     * Find the index into the Shadow Counter table
     */
    if( SB_QE2000_ELIB_VIT_CMAP_MAP0 == pVrtMap->tCmap )
    {
        nCountFwdIdx = pEp->nCMapClassCnt[0];
    }
    else
    {
        nCountFwdIdx = pEp->nCMapClassCnt[1];
    }

    /*
     * Index past the Class Counts
     */
    nPortIdx += nCountFwdIdx;

    pCounts->ullPktCount  = pVrtMap->pCntrShadow[nPortIdx].ullPktCount;
    pCounts->ullByteCount = pVrtMap->pCntrShadow[nPortIdx].ullByteCount;
    if(TRUE == bClrOnRd)
    {
        COMPILER_64_ZERO(pVrtMap->pCntrShadow[nPortIdx].ullPktCount);
        COMPILER_64_ZERO(pVrtMap->pCntrShadow[nPortIdx].ullByteCount);
    }

    return( SB_ELIB_OK );
}

sbElibStatus_et sbQe2000ElibVlanCountGetAll( SB_QE2000_ELIB_HANDLE Handle, uint32 ulScIdx,
                                 bool_t bClrOnRd,
                                 SB_QE2000_ELIB_COUNTER_ST aCounts[SB_QE2000_ELIB_MAX_COUNTERS_K])
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    SB_QE2000_ELIB_VRT_MAP_ENTRY_PST pVrtMap;
    int i;
    int nCounts;

    SB_ASSERT( Handle );
    SB_ASSERT( pEp->pHalCtx );
    if (ulScIdx >= SBQE2000_ELIB_VRT_MAP_MAX_K) {
        return ( SB_ELIB_BAD_ARGS);
    }

    pVrtMap = pEp->apVrtMap[ulScIdx];

    if( NULL == pVrtMap )
    {
        return( SB_ELIB_BAD_IDX );
    }

    /*
     * Account for the Port Control Word
     */
    if( 0 == pEp->nOnePcwEnable )
    {
        nCounts = pVrtMap->nNumLines - 2;
    }
    else
    {
        nCounts = pVrtMap->nNumLines - 1;
    }

    /* Address DEFECT 22280, was not taking into account the possible dummy line */
    if(TRUE == pVrtMap-> bDummyLine)
    {
        nCounts--;
    }

    for( i = 0; i < nCounts; i++)
    {
        aCounts[i].ullPktCount  = pVrtMap->pCntrShadow[i].ullPktCount;
        aCounts[i].ullByteCount = pVrtMap->pCntrShadow[i].ullByteCount;
        if(TRUE == bClrOnRd)
        {
            COMPILER_64_ZERO(pVrtMap->pCntrShadow[i].ullPktCount);
            COMPILER_64_ZERO(pVrtMap->pCntrShadow[i].ullByteCount);
        }
    }

    return( SB_ELIB_OK );
}

sbElibStatus_et sbQe2000ElibVlanCounterSetsAccumulate( SB_QE2000_ELIB_HANDLE Handle, uint32 *pulNumCounters )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    sbElibStatus_et nStatus;
    int nLastScIdx;
    uint32 ulNumCounters;
    uint32 ulCounts;

    SB_ASSERT( Handle );
    SB_ASSERT( pEp->pHalCtx );
    SB_ASSERT( pulNumCounters );

    nLastScIdx = pEp->nLastScIdxAccum;
    ulNumCounters = *pulNumCounters;
    *pulNumCounters = 0;

    do
    {
        if(SBQE2000_ELIB_VRT_MAP_MAX_K == nLastScIdx)
        {
            nLastScIdx = 0;
        }

        if(NULL != pEp->apVrtMap[nLastScIdx])
        {
            ulCounts = 0;
            nStatus = sbQe2000ElibScIdxCountAccumulate(pEp, nLastScIdx, &ulCounts);
            if( SB_ELIB_OK != nStatus )
            {
                pEp->nLastScIdxAccum = nLastScIdx;
                return( nStatus );
            }
            else
            {
                *pulNumCounters += ulCounts;
            }
        }
        nLastScIdx++;

        if(nLastScIdx == pEp->nLastScIdxAccum)
        {
            /*
             * We've wrapped, no need to continue to accumulate
             * counters, get out.
             */
            break;
        }

    } while(*pulNumCounters < ulNumCounters);


    pEp->nLastScIdxAccum = nLastScIdx;

    return( SB_ELIB_OK );

}

sbElibStatus_et sbQe2000ElibVlanCountAccumulate( SB_QE2000_ELIB_HANDLE Handle, uint32 ulNumScIdx, uint32 *pulCounts)
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    sbElibStatus_et nStatus;
    uint32 ulCounts;
    int nLastScIdx;
    int nScIdx;

    SB_ASSERT( Handle );
    SB_ASSERT( pEp->pHalCtx );
    SB_ASSERT( pulCounts );

    *pulCounts = 0;
    ulCounts = 0;

    nLastScIdx = pEp->nLastScIdxAccum;

    for(nScIdx = 0; nScIdx < ulNumScIdx; nScIdx++)
    {
        if(SBQE2000_ELIB_VRT_MAP_MAX_K == nLastScIdx)
        {
            nLastScIdx = 0;
        }

        if(NULL != pEp->apVrtMap[nLastScIdx])
        {
            nStatus = sbQe2000ElibScIdxCountAccumulate(pEp, nLastScIdx, &ulCounts);
            if( 0 != nStatus )
            {
                pEp->nLastScIdxAccum = nLastScIdx;
                return( nStatus );
            }
            else
            {
                *pulCounts += ulCounts;
            }
        }
        nLastScIdx++;

        if(nLastScIdx == pEp->nLastScIdxAccum)
        {
            /*
             * We've wrapped, no need to continue to accumulate
             * counters, get out.
             */
            break;
        }

    }

    pEp->nLastScIdxAccum = nLastScIdx;

    return( SB_ELIB_OK );
}


static sbElibStatus_et sbQe2000ElibScIdxCountAccumulate( SB_QE2000_ELIB_CONTEXT_PST pEp, uint32 ulScIdx, uint32 *pulCounts )
{
    SB_QE2000_ELIB_VRT_MAP_ENTRY_PST pVrtMap;
    SB_QE2000_ELIB_DMA_FUNC_PARAMS_ST sDmaParams;
    sbZfSbQe2000ElibPCTSingle_t sZfCount;
    uint64 ullPopMask;
    uint32 aulCntr[(SB_QE2000_ELIB_NUM_PORTS_K + SB_QE2000_ELIB_NUM_CLASSES_K) * 2];
    VrtPtr_t tVrtPtr;
    int nMemIdx;
    int i;
    sbElibStatus_et nStatus;
    int nNumLines;
    int nCounts;
    int nNumLinesFirstPCW, nNumLinesSecondPCW;
    int nCmap;

    SB_ASSERT( pEp->pHalCtx );
    SB_ASSERT( pulCounts );

    pVrtMap = pEp->apVrtMap[ulScIdx];

    if( NULL == pVrtMap )
    {
        return( SB_ELIB_BAD_IDX );
    }

    /*
     * Address DEFECT 22280:  Clear counter landing pad
     */
    SB_MEMSET(aulCntr, 0, sizeof(uint32) * (SB_QE2000_ELIB_NUM_PORTS_K + SB_QE2000_ELIB_NUM_CLASSES_K) * 2);

    /*
     * Figure out how many counts to accumulate
     */
    if( (SB_QE2000_ELIB_VIT_CMAP_0_NO_PORT == pVrtMap->tCmap) ||
        (SB_QE2000_ELIB_VIT_CMAP_MAP0 == pVrtMap->tCmap) )
    {
        nNumLines = pVrtMap->nNumLines;
        nCmap = 0;
    }
    else
    {
        nNumLines = pVrtMap->nNumLines;
        nCmap = 1;
    }

    /*
     * Account for the Port Control Word
     */
    if( 0 == pEp->nOnePcwEnable )
    {
        tVrtPtr = pVrtMap->tVrtPtr + 2;
        nNumLines -= 2;
    }
    else
    {
        tVrtPtr = pVrtMap->tVrtPtr + 1;
        nNumLines--;
    }

    nCounts = nNumLines;
    if( TRUE == pVrtMap->bDummyLine )
    {
        nCounts--;
    }


    if( (nNumLines + tVrtPtr ) > SB_QE2000_ELIB_BF_MEM_MAX_OFFSET )
    {
        *pulCounts = 0;
        return( SB_ELIB_BAD_IDX );
    }

    /*
     * Get all the counts
     *
     * If we have a user DMA function, and we're obtaining more than 3
     * lines (minimum of 192 bytes) use the user DMA function.
     *
     * Otherwise, use PIO to get the counters.
     */
    if( (NULL != pEp->pfUserDmaFunc) && (3 < nNumLines) )
    {
        /*
         * Use DMA to get the counters
         */
        SB_MEMSET(&sDmaParams, 0, sizeof(sDmaParams));
        sDmaParams.StatsMem = SB_QE2000_ELIB_STATS_VRT;
        sDmaParams.ulStartAddr = tVrtPtr;
        sDmaParams.ulNumLines = nNumLines;
        sDmaParams.ulPreserve = 0; /* Clear on Read */
        sDmaParams.pData = (void *)&aulCntr[0];
        sDmaParams.pUserData = pEp->pUserData; /* Pass the user supplied data back to them */

        nStatus = (*pEp->pfUserDmaFunc)((sbhandle)pEp->pHalCtx, &sDmaParams);
        if( SB_ELIB_OK != nStatus )
        {
            return( SB_ELIB_DMA_FAIL );
        }
    }
    else
    {
        nMemIdx = 0;
        for(i = 0; i < nNumLines; i++)
        {
            nStatus = sbQe2000ElibBfMemRead( pEp->pHalCtx,
                                             tVrtPtr + i,
                                             1, /* Clear on Read */
                                             &aulCntr[nMemIdx],
                                             &aulCntr[nMemIdx + 1] );
            if( SB_ELIB_OK != nStatus )
            {
                return( nStatus );
            }
            nMemIdx += 2;
        }
    }


    nMemIdx = 0;

    /*
     * Update the class counters first
     */
    nNumLines = pEp->nCMapClassCnt[nCmap];
    for( i = 0; i < nNumLines; i++)
    {
        /*
         * Update the shadow count with the counters
         * NOTE: In the case
         */
        sbZfSbQe2000ElibPCTSingle_Unpack(&sZfCount, (uint8 *)&aulCntr[nMemIdx], 0);
        COMPILER_64_ADD_64(pVrtMap->pCntrShadow[i].ullPktCount, sZfCount.m_PktClass);
        COMPILER_64_ADD_64(pVrtMap->pCntrShadow[i].ullByteCount, sZfCount.m_ByteClass);
        nMemIdx += 2;
    }

    if( (SB_QE2000_ELIB_VIT_CMAP_0_NO_PORT == pVrtMap->tCmap) ||
        (SB_QE2000_ELIB_VIT_CMAP_1_NO_PORT == pVrtMap->tCmap) )
    {
        *pulCounts = nCounts;
        return( SB_ELIB_OK );
    }

    /*
     * Now accumulate the port counts
     * 1st PCW counts
     * ullPopMask = (1 << 24) - 1;
     * ullPopMask |= ( ((uint64)1 << 48) | ((uint64)1 << 49) );
     */

    COMPILER_64_SET(ullPopMask, 3 << 16, ((1 << 24) - 1));
    COMPILER_64_AND(ullPopMask,pVrtMap->ullPortEnable); 
    nNumLinesFirstPCW = sbQe2000ElibGetPopulationCount(ullPopMask);

    for(i = nNumLines; i < nNumLinesFirstPCW + nNumLines; i++)
    {
        sbZfSbQe2000ElibPCTSingle_Unpack(&sZfCount, (uint8 *)&aulCntr[nMemIdx], 0);
        COMPILER_64_ADD_64(pVrtMap->pCntrShadow[i].ullPktCount, sZfCount.m_PktClass);
        COMPILER_64_ADD_64(pVrtMap->pCntrShadow[i].ullByteCount, sZfCount.m_ByteClass);
        nMemIdx += 2;
    }

    if( TRUE == pVrtMap->bDummyLine )
    {
        nMemIdx += 2;
    }

    /*
     * 2nd PCW Counts
     */
    if( 0 == pEp->nOnePcwEnable )
    {
        /* ullPopMask =  ~(((uint64)1 << 48) | ((uint64)1 << 49) | ((1 << 24) - 1)); */
        COMPILER_64_SET(ullPopMask, 3 << 16, ((1<<24) -1));
        COMPILER_64_NOT(ullPopMask);
        COMPILER_64_AND(ullPopMask,pVrtMap->ullPortEnable); 
        nNumLinesSecondPCW = sbQe2000ElibGetPopulationCount(ullPopMask);
        /* Address DEFECT 2280 */
        for(i = nNumLines + nNumLinesFirstPCW; i < nNumLinesFirstPCW + nNumLinesSecondPCW + nNumLines; i++)
        {
            sbZfSbQe2000ElibPCTSingle_Unpack(&sZfCount, (uint8 *)&aulCntr[nMemIdx], 0);
            COMPILER_64_ADD_64(pVrtMap->pCntrShadow[i].ullPktCount, sZfCount.m_PktClass);
            COMPILER_64_ADD_64(pVrtMap->pCntrShadow[i].ullByteCount, sZfCount.m_ByteClass);
            nMemIdx += 2;
        }
    }

    *pulCounts = nCounts;
    return( SB_ELIB_OK );
}

sbElibStatus_et sbQe2000ElibVlanCountReset( SB_QE2000_ELIB_CONTEXT_PST pEp )
{
    SB_QE2000_ELIB_VRT_MAP_ENTRY_PST pVrtMap;
    uint32 ulScIdx;
    int nCounts;
    int nNumLines;
    int nIdx;

    for( ulScIdx = 0; ulScIdx <  SBQE2000_ELIB_SCIDX_MAX_K; ulScIdx++ )
    {
        pVrtMap = pEp->apVrtMap[ulScIdx];
        if( NULL == pVrtMap )
        {
            continue;
        }

        /*
         * Figure out how many counts to clear
         */
        
        nNumLines = pVrtMap->nNumLines;
        

        /*
         * Account for the Port Control Word
         */
        if( 0 == pEp->nOnePcwEnable )
        {
            nNumLines -= 2;
        }
        else
        {
            nNumLines--;
        }

        nCounts = nNumLines;
        if( TRUE == pVrtMap->bDummyLine )
        {
            nCounts--;
        }

        for( nIdx = 0; nIdx < nCounts; nIdx++ )
        {
            COMPILER_64_ZERO(pVrtMap->pCntrShadow[nIdx].ullPktCount);
            COMPILER_64_ZERO(pVrtMap->pCntrShadow[nIdx].ullByteCount);
        }
    }

    return( SB_ELIB_OK );

}
