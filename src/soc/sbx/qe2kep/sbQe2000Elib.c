/*
 *
 * =========================================
 * ==  sbQe2000Elib.c - elib main module  ==
 * =========================================
 *
 * WORKING REVISION: $Id: sbQe2000Elib.c,v 1.18 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * MODULE NAME:
 *
 *     sbQe2000Elib.c
 *
 * ABSTRACT:
 *
 *     elib public API
 *
 * LANGUAGE:
 *
 *     C
 *
 * AUTHORS:
 *
 *     David R. Pickett
 *     Travis B. Sawyer
 *
 * CREATION DATE:
 *
 *     20-August-2004
 *
 */
#include "sbWrappers.h"
#include "sbTypesGlue.h"
#include "sbQe2000Elib.h"
#include "sbTypes.h"
#include "sbQe2000ElibMem.h"
#include "sbQe2000ElibContext.h"
#include "sbQe2000ElibZf.h"
#include "sbElibStatus.h"
#include "soc/drv.h"

#define SB_QE2000_EP_BIST_RUN

/*
 * Static function prototypes
 */
static sbElibStatus_et sbQe2000ElibClassPktCountPoll( SB_QE2000_ELIB_CONTEXT_PST pEp );
static sbElibStatus_et sbQe2000ElibPCTPoll( SB_QE2000_ELIB_CONTEXT_PST pEp );
static sbElibStatus_et sbQe2000ElibPCTDmaPoll( SB_QE2000_ELIB_CONTEXT_PST pEp, uint8 *pucPCT );
static sbElibStatus_et sbQe2000ElibPCTPioPoll( SB_QE2000_ELIB_CONTEXT_PST pEp, uint8 *pucPCT );
#if defined(SB_QE2000_EP_BIST_RUN)
static sbElibStatus_et sbQe2000ElibBistRun( SB_QE2000_ELIB_CONTEXT_PST pEp, uint32 *pulStatus );
#endif


/*
 * Public API
 */

sbElibStatus_et sbQe2000ElibDefParamsGet( SB_QE2000_ELIB_INIT_PARAMS_PST pParams )
{
    SB_ASSERT( pParams );
    DENTER();
    SB_MEMSET(pParams, 0, sizeof(*pParams));
    pParams->bSoftClassEna = TRUE;
    pParams->bVitEna = TRUE;
    pParams->bOnePcwEna = FALSE;
    pParams->bBypassBF = FALSE;
    pParams->bBypassIP = FALSE;
    pParams->bBypassCL = FALSE;
    pParams->bEpEna = TRUE;
    COMPILER_64_SET(pParams->ullERHForward, 0x20000, 0); /* PCI always gets ERH */
    COMPILER_64_ZERO(pParams->ullSmacMsbEnable);
    COMPILER_64_ZERO(pParams->ullMirrorEnable);
    pParams->MmConfig = SB_QE2000_ELIB_MM_CONFIG_BF16;
    pParams->pfUserDmaFunc = NULL;
    pParams->pUserData = NULL;
    pParams->nPorts = SB_QE2000_ELIB_NUM_PORTS_K;
    pParams->pfUserSemCreate = NULL;
    pParams->pfUserSemTryWait = NULL;
    pParams->pfUserSemGive = NULL;
    pParams->pfUserSemDestroy = NULL;
    pParams->pUserSemData = NULL;
    pParams->pMVTUserSemData = NULL;
    pParams->nMVTSemId = 0;
    pParams->nMVTTimeOut = 0;
    DEXIT();
    return( SB_ELIB_OK );
}



/* coverity[pass_by_value] */
sbElibStatus_et sbQe2000ElibInit( SB_QE2000_ELIB_HANDLE *pHandle, void *pHalCtx, SB_QE2000_ELIB_INIT_PARAMS_ST Params )
{
    SB_QE2000_ELIB_CONTEXT_PST pEpContext;
    sbDmaMemoryHandle_t tDmaHdl;
    uint32 ulEpConfigReg;
    uint32 ulEpClReg;
    uint32 ulEpBfReg;
    uint32 ulEpMmReg;
    uint32 ulERH0;
    uint32 ulERH1;
    uint32 ulSmacMsb0;
    uint32 ulSmacMsb1;
    uint32 ulMirror0;
    uint32 ulMirror1;
    uint32 ulVencapTag;
    sbElibStatus_et nStatus;
#if defined(SB_QE2000_EP_BIST_RUN)
    uint32 ulBistStatus;
#endif
    void *pTmpVoid;

    SB_ASSERT( Params.MmConfig < SB_QE2000_ELIB_MM_CONFIG_MAX );

    DENTER();

    pTmpVoid = NULL;
    nStatus = (sbElibStatus_et) gen_thin_malloc((void *)pHalCtx,
                              SB_ALLOC_INTERNAL,
                              sizeof(SB_QE2000_ELIB_CONTEXT_ST),
                              (void **)&pTmpVoid,
                              NULL,
                              0);

    if( SB_ELIB_OK != nStatus )
    {
        DEXIT();
        return( SB_ELIB_MEM_ALLOC_FAIL );
    }

    pEpContext = (SB_QE2000_ELIB_CONTEXT_ST *)pTmpVoid;

    ulERH0 = 0;
    ulERH1 = 0;
    ulSmacMsb0 = 0;
    ulSmacMsb1 = 0;
    ulMirror0 = 0;
    ulMirror1 = 0;

    SB_MEMSET(pEpContext, 0, sizeof(*pEpContext));

    /*
     * Save off the HAL context for later use.
     */
    pEpContext->pHalCtx = pHalCtx;
    pEpContext->pUserData = Params.pUserData;
    pEpContext->pfUserDmaFunc = Params.pfUserDmaFunc;
    pEpContext->nLastScIdxAccum = 0;
    pEpContext->nPorts = Params.nPorts;
    pEpContext->nMVTSemId = Params.nMVTSemId;
    pEpContext->nMVTTimeOut = Params.nMVTTimeOut;
    pEpContext->pMVTUserSemData = Params.pMVTUserSemData;
    ulEpConfigReg = 0;

    /*
     * Clear the local shadowed tables
     */
    SB_MEMSET(pEpContext->abCRTDirty, FALSE, sizeof(pEpContext->abCRTDirty));

    if (TRUE == Params.bEpEna)
    {
#if defined(SB_QE2000_EP_BIST_RUN)
        /* Run BIST on the EP */
        nStatus = sbQe2000ElibBistRun( pEpContext, &ulBistStatus );
        if( SB_ELIB_OK != nStatus )
        {
            /*
             * We have some error from running bist.
             */
            *pHandle = NULL;
            tDmaHdl.handle = NULL;
            thin_free((void *)pHalCtx,
                      SB_ALLOC_INTERNAL,
                      0, /* don't care */
                      (void*)pEpContext,
                      tDmaHdl);
            DEXIT();
            return( nStatus );
        }
#endif

        /*
         * If the user semaphore functions are non-null, attempt to create
         * the stats access semaphore
         */
        if( (NULL != Params.pfUserSemCreate) && (NULL != Params.pfUserSemTryWait) &&
            (NULL != Params.pfUserSemGive) )
        {
            nStatus = (*Params.pfUserSemCreate)(Params.pUserSemData);
            if( -1 == nStatus )
            {
                /*
                 * We've failed to create the semaphore, but it isn't
                 * catastrophic.  We can continue on, but statistics
                 * access will not be guarded
                 */
                pEpContext->pfUserSemCreate  = NULL;
                pEpContext->pfUserSemTryWait = NULL;
                pEpContext->pfUserSemGive    = NULL;
                pEpContext->pUserSemData     = NULL;
                pEpContext->pfUserSemDestroy = NULL;
                pEpContext->nStatsSemId      = -1;
            }
            else
            {
                pEpContext->pfUserSemCreate  = Params.pfUserSemCreate;
                pEpContext->pfUserSemTryWait = Params.pfUserSemTryWait;
                pEpContext->pfUserSemGive    = Params.pfUserSemGive;
                pEpContext->pUserSemData     = Params.pUserSemData;
                pEpContext->pfUserSemDestroy = Params.pfUserSemDestroy;
                pEpContext->nStatsSemId      = nStatus;

            }
        }
        else
        {
            pEpContext->pfUserSemCreate  = NULL;
            pEpContext->pfUserSemTryWait = NULL;
            pEpContext->pfUserSemGive    = NULL;
            pEpContext->pUserSemData     = NULL;
            pEpContext->pfUserSemDestroy = NULL;
            pEpContext->nStatsSemId      = -1;
        }

        /*
         * Retrieve the current settings for registers to be written
         */
        ulEpBfReg = SAND_HAL_READ(pHalCtx, KA, EP_BF_CONFIG);

        /*
         * Set the Classifier SoftClass per user
         */
        if (TRUE == Params.bSoftClassEna)
        {
            ulEpClReg = SAND_HAL_SET_FIELD(KA, EP_CL_ENABLE, SOFT_CLASS, 0xFF);
        }
        else
        {
            ulEpClReg = SAND_HAL_SET_FIELD(KA, EP_CL_ENABLE, SOFT_CLASS, 0x00);
        }

        /*
         * Set the Bridging Function according to the user
         */
        if (TRUE == Params.bBypassBF)
        {
            ulEpConfigReg |= SAND_HAL_SET_FIELD(KA, EP_CONFIG, BYPASS_BF, 1);
        }
        else
        {
            /*
             * Setup the Bridging Function Configuration
             */
            if (TRUE == Params.bVitEna)
            {
                ulEpBfReg = SAND_HAL_MOD_FIELD(KA, EP_BF_CONFIG, VIT_ENABLE, ulEpBfReg, 1);
            }
            else
            {
                ulEpBfReg = SAND_HAL_MOD_FIELD(KA, EP_BF_CONFIG, VIT_ENABLE, ulEpBfReg, 0);
            }

            if (TRUE == Params.bOnePcwEna)
            {
                ulEpBfReg = SAND_HAL_MOD_FIELD(KA, EP_BF_CONFIG, PCW1_ENABLE, ulEpBfReg, 1);
                pEpContext->nOnePcwEnable = 1;
            }
            else
            {
                ulEpBfReg = SAND_HAL_MOD_FIELD(KA, EP_BF_CONFIG, PCW1_ENABLE, ulEpBfReg, 0);
                pEpContext->nOnePcwEnable = 0;
            }

            ulERH0 = COMPILER_64_LO(Params.ullERHForward);
            ulERH1 = COMPILER_64_HI(Params.ullERHForward) & 0x3FFFF;
            ulSmacMsb0 = COMPILER_64_LO(Params.ullSmacMsbEnable);
            ulSmacMsb1 = COMPILER_64_HI(Params.ullSmacMsbEnable) & 0x3FFFF;
            ulMirror0 = COMPILER_64_LO(Params.ullMirrorEnable);
            ulMirror1 = COMPILER_64_HI(Params.ullMirrorEnable) & 0x3FFFF;
        }

        /*
         * Set the IP function enable according to the user
         */
        if (TRUE == Params.bBypassIP)
        {
            ulEpConfigReg |= SAND_HAL_SET_FIELD(KA, EP_CONFIG, BYPASS_IP, 1);
        }

        /*
         * Set the Classifier function enable according to the user
         */
        if (TRUE == Params.bBypassCL)
        {
            ulEpConfigReg |= SAND_HAL_SET_FIELD(KA, EP_CONFIG, BYPASS_CL, 1);
        }

        /*
         * Set the Memory Management Configuration
         */
        ulEpMmReg = SAND_HAL_SET_FIELD(KA, EP_MM_CONFIG, ENABLE, Params.MmConfig);

        /*
         * Enable the EP
         */
        ulEpConfigReg |= SAND_HAL_SET_FIELD(KA, EP_CONFIG, ENABLE, 1);

        /*
         * Fire all of the config settings into the QE2K
         */
        SAND_HAL_WRITE(pHalCtx, KA, EP_BF_CONFIG, ulEpBfReg);
        SAND_HAL_WRITE(pHalCtx, KA, EP_CL_ENABLE, ulEpClReg);
        SAND_HAL_WRITE(pHalCtx, KA, EP_MM_CONFIG, ulEpMmReg);
        SAND_HAL_WRITE(pHalCtx, KA, EP_CONFIG, ulEpConfigReg);
        SAND_HAL_WRITE(pHalCtx, KA, EP_BF_SMAC_MC0, ulSmacMsb0);
        SAND_HAL_WRITE(pHalCtx, KA, EP_BF_SMAC_MC1, ulSmacMsb1);
#if 1
	/* arjb 080229: this is now initialized during soc_init */
	(void)ulERH0;
	(void)ulERH1;
	(void)ulMirror0;
	(void)ulMirror1;
#else
        SAND_HAL_WRITE(pHalCtx, KA, EP_BF_ERH0, ulERH0);
        SAND_HAL_WRITE(pHalCtx, KA, EP_BF_ERH1, ulERH1);
        SAND_HAL_WRITE(pHalCtx, KA, EP_BF_PORT_MIRROR0, ulMirror0);
        SAND_HAL_WRITE(pHalCtx, KA, EP_BF_PORT_MIRROR1, ulMirror1);
#endif
        ulVencapTag = SAND_HAL_READ(pHalCtx, KA, EP_IP_VENCAP_TAG);
        if( 0 != Params.ulVlanA )
        {
            ulVencapTag = SAND_HAL_MOD_FIELD( KA, EP_IP_VENCAP_TAG, A, ulVencapTag, Params.ulVlanA );
        }
        if( 0 != Params.ulVlanB )
        {
            ulVencapTag = SAND_HAL_MOD_FIELD( KA, EP_IP_VENCAP_TAG, B, ulVencapTag, Params.ulVlanB );
        }
        SAND_HAL_WRITE(pHalCtx, KA, EP_IP_VENCAP_TAG, ulVencapTag);

#ifndef BACKDOOR_CLEAR
        /* Clear out (zero) the QE2k-Ep memories */
        nStatus = sbQe2000ElibMemClear(pEpContext);
        if( nStatus != 0 )
        {
            return nStatus;
        }
#endif

        /*
         * Initialize the Vlan Memory Manager
         */
        nStatus = sbQe2000ElibVlanMemInit(pEpContext);
        if(SB_ELIB_OK != nStatus)
        {
            DEXIT();
            return( nStatus );
        }

        /*
         * Create the default VIT direct entry for the PCI path.
         */
        nStatus = sbQe2000ElibVITPciSet(pEpContext);
        if(SB_ELIB_OK != nStatus)
        {
            DEXIT();
            return( nStatus );
        }
    }
    else
    {
        /*
         * Parameters dictate that the EP be turned off
         */
        SAND_HAL_RMW_FIELD(pHalCtx, KA, EP_CONFIG, ENABLE, 0);
    }

    *pHandle = (SB_QE2000_ELIB_HANDLE *)pEpContext;

    DEXIT();
    return (0);

}

sbElibStatus_et sbQe2000ElibUnInit( SB_QE2000_ELIB_HANDLE Handle )
{

    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    sbDmaMemoryHandle_t tDmaHdl;
    sbElibStatus_et nStatus;

    DENTER();
    SB_ASSERT( Handle );

    /*
     * Turn off the EP
     */
    SAND_HAL_RMW_FIELD(pEp->pHalCtx, KA, EP_CONFIG, ENABLE, 0);

    /*
     * Kill our stats semaphore
     */
    if(NULL != pEp->pfUserSemDestroy)
    {
        (*pEp->pfUserSemDestroy)(pEp->nStatsSemId, pEp->pUserSemData);
    }

    /*
     * Uninitialize the Switch Context Record Memory Manager
     */
    nStatus = sbQe2000ElibVlanMemUninit(pEp);

    /*
     * Destroy and poison our handle
     */
    SB_MEMSET((void *)Handle, 5, sizeof(SB_QE2000_ELIB_CONTEXT_ST));
    tDmaHdl.handle = NULL;
    thin_free((void *)pEp->pHalCtx,
              SB_ALLOC_INTERNAL,
              0, /* don't care */
              (void*)Handle,
              tDmaHdl);

    Handle = NULL;

    DEXIT();
    return( nStatus );
}


sbElibStatus_et sbQe2000ElibVlanEncapSet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulVlanTagA, uint32 ulVlanTagB )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    uint32 ulVencapTag;

    DENTER();
    SB_ASSERT( Handle );

    ulVencapTag = SAND_HAL_READ(pEp->pHalCtx, KA, EP_IP_VENCAP_TAG);
    ulVencapTag = SAND_HAL_MOD_FIELD( KA, EP_IP_VENCAP_TAG, A, ulVencapTag, ulVlanTagA );
    ulVencapTag = SAND_HAL_MOD_FIELD( KA, EP_IP_VENCAP_TAG, B, ulVencapTag, ulVlanTagB );

    SAND_HAL_WRITE(pEp->pHalCtx, KA, EP_IP_VENCAP_TAG, ulVencapTag);

    return( SB_ELIB_OK );

}

sbElibStatus_et sbQe2000ElibVlanEncapGet( SB_QE2000_ELIB_HANDLE Handle, uint32 *pulVlanTagA, uint32 *pulVlanTagB )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    uint32 ulVencapTag;

    DENTER();
    SB_ASSERT( Handle );
    SB_ASSERT( pulVlanTagA );
    SB_ASSERT( pulVlanTagB );

    ulVencapTag = SAND_HAL_READ(pEp->pHalCtx, KA, EP_IP_VENCAP_TAG);

    *pulVlanTagA = SAND_HAL_GET_FIELD(KA, EP_IP_VENCAP_TAG, A, ulVencapTag);
    *pulVlanTagB = SAND_HAL_GET_FIELD(KA, EP_IP_VENCAP_TAG, B, ulVencapTag);

    return( SB_ELIB_OK );

}

sbElibStatus_et sbQe2000ElibMplsEncapSet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulMpls )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    uint32 ulMplsEncap;

    DENTER();
    SB_ASSERT( Handle );

    ulMplsEncap = SAND_HAL_SET_FIELD(KA, EP_IP_UEENCAP_TYPE0, ETYPE, ulMpls);

    SAND_HAL_WRITE(pEp->pHalCtx, KA, EP_IP_UEENCAP_TYPE0, ulMplsEncap);

    return( SB_ELIB_OK );

}

sbElibStatus_et sbQe2000ElibMplsEncapGet( SB_QE2000_ELIB_HANDLE Handle, uint32 *pulMpls )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    uint32 ulMplsEncap;

    DENTER();
    SB_ASSERT( Handle );

    ulMplsEncap = SAND_HAL_READ(pEp->pHalCtx, KA, EP_IP_UEENCAP_TYPE0);
    *pulMpls = SAND_HAL_GET_FIELD(KA, EP_IP_UEENCAP_TYPE0, ETYPE, ulMplsEncap);

    return( SB_ELIB_OK );

}

sbElibStatus_et sbQe2000ElibPortCfgSet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulPort, SB_QE2000_ELIB_PORT_CFG_ST Cfg )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    sbElibStatus_et nStatus;
    SB_QE2000_ELIB_PORT_CFG_ST oldCfg;
    uint32 ulRewrite, ulBf;
    uint32 ulUntag, ulTagSel;
    uint32 ulModPort;
    sbZfSbQe2000ElibPT_t sZfPt;
    uint32 aulData[SB_ZF_SB_QE2000_ELIB_PT_ENTRY_SIZE_IN_BYTES / sizeof(uint32)];
    uint32 ulClTransitPkts, ulClTransitBytes;
    int i;

    DENTER();
    SB_ASSERT( Handle );
    SB_ASSERT( ulPort < SB_QE2000_ELIB_NUM_PORTS_K );

    /* coverity[unsigned_compare] */
    if ( (0 <= ulPort) && (32 > ulPort) )
    {
        /*
         * Reg Bit == Port
         */
        ulModPort = ulPort;

        ulRewrite  = SAND_HAL_READ(pEp->pHalCtx, KA, EP_BF_PRI_REWRITE0);
        ulBf       = SAND_HAL_READ(pEp->pHalCtx, KA, EP_BF_PORT0);
        ulUntag    = SAND_HAL_READ(pEp->pHalCtx, KA, EP_BF_TAG0);
        ulTagSel   = SAND_HAL_READ(pEp->pHalCtx, KA, EP_IP_EENCAP_TAG0);
    }
    else
    {
        /*
         * Reg Bit == Port - 32
         */
        ulModPort = ulPort - 32;

        ulRewrite  = SAND_HAL_READ(pEp->pHalCtx, KA, EP_BF_PRI_REWRITE1);
        ulBf       = SAND_HAL_READ(pEp->pHalCtx, KA, EP_BF_PORT1);
        ulUntag    = SAND_HAL_READ(pEp->pHalCtx, KA, EP_BF_TAG1);
        ulTagSel   = SAND_HAL_READ(pEp->pHalCtx, KA, EP_IP_EENCAP_TAG1);
    }


    ulRewrite = Cfg.bPriRewriteEna ? ( ulRewrite | ( 1 << ulModPort ) ) :
        ( ulRewrite & ~( 1 << ulModPort ) );

    /*
     * [Bug 2471] BF Must be bypassed for PCI Port due to stuffit
     * NOTE: Bug 2471 surpassed by need for Class Instructions to forward
     * NOTE: to/from the PCI path.
     */
    if( 49 == ulPort )
    {
        /*
         * Ensure BF is ON for port 49, the PCI port
         */
        ulBf |= (1 << ulModPort);
    }
    else
    {
        ulBf = Cfg.bBfEna ? ( ulBf | ( 1 << ulModPort ) ) :
            ( ulBf & ~( 1 << ulModPort ) );
    }

    ulUntag = Cfg.bUntaggedEna ? ( ulUntag | ( 1 << ulModPort ) ) :
        ( ulUntag & ~( 1 << ulModPort ) );

    ulTagSel = ( SB_QE2000_ELIB_EETAG_B == Cfg.TagSel ) ? ( ulTagSel | ( 1 << ulModPort ) ) :
        ( ulTagSel & ~( 1 << ulModPort ) );

    /*
     * Write out the new configuration
     */
    if ( (0 <= ulPort) && (32 > ulPort) )
    {
        SAND_HAL_WRITE( pEp->pHalCtx, KA, EP_BF_PRI_REWRITE0, ulRewrite  );
        SAND_HAL_WRITE( pEp->pHalCtx, KA, EP_BF_PORT0,        ulBf       );
        SAND_HAL_WRITE( pEp->pHalCtx, KA, EP_BF_TAG0,         ulUntag    );
        SAND_HAL_WRITE( pEp->pHalCtx, KA, EP_IP_EENCAP_TAG0,  ulTagSel   );
    }
    else
    {
        SAND_HAL_WRITE( pEp->pHalCtx, KA, EP_BF_PRI_REWRITE1, ulRewrite  );
        SAND_HAL_WRITE( pEp->pHalCtx, KA, EP_BF_PORT1,        ulBf       );
        SAND_HAL_WRITE( pEp->pHalCtx, KA, EP_BF_TAG1,         ulUntag    );
        SAND_HAL_WRITE( pEp->pHalCtx, KA, EP_IP_EENCAP_TAG1,  ulTagSel   );
    }

    /*
     * NOTE:  To do multiple class transitions for a
     * NOTE:  port, multiple calls to sbQe2000ElibPortCfgSet
     * NOTE:  would need to occur.
     * NOTE:  instead, we scan through the class enable
     * NOTE:  array and transition each class accordingly
     */
    nStatus = sbQe2000ElibPortCfgGet( Handle, ulPort, &oldCfg );
    if( SB_ELIB_OK != nStatus )
    {
        DEXIT();
        return( SB_ELIB_PORT_CFG_GET_FAIL );
    }

    /*
     * Start with the old config and transition
     * each class as appropriate.
     */
    sbZfSbQe2000ElibPT_InitInstance(&sZfPt);
    sZfPt.m_bClassEnb0 = oldCfg.bClassEna[0];
    sZfPt.m_bClassEnb1 = oldCfg.bClassEna[1];
    sZfPt.m_bClassEnb2 = oldCfg.bClassEna[2];
    sZfPt.m_bClassEnb3 = oldCfg.bClassEna[3];
    sZfPt.m_bClassEnb4 = oldCfg.bClassEna[4];
    sZfPt.m_bClassEnb5 = oldCfg.bClassEna[5];
    sZfPt.m_bClassEnb6 = oldCfg.bClassEna[6];
    sZfPt.m_bClassEnb7 = oldCfg.bClassEna[7];
    sZfPt.m_bClassEnb8 = oldCfg.bClassEna[8];
    sZfPt.m_bClassEnb9 = oldCfg.bClassEna[9];
    sZfPt.m_bClassEnb10 = oldCfg.bClassEna[10];
    sZfPt.m_bClassEnb11 = oldCfg.bClassEna[11];
    sZfPt.m_bClassEnb12 = oldCfg.bClassEna[12];
    sZfPt.m_bClassEnb13 = oldCfg.bClassEna[13];
    sZfPt.m_bClassEnb14 = oldCfg.bClassEna[14];
    sZfPt.m_bClassEnb15 = oldCfg.bClassEna[15];

    sZfPt.m_Instruction = Cfg.ulInst;
    sZfPt.m_bInstValid  = Cfg.bInstValid;
    sZfPt.m_bPrepend    = Cfg.bAppend;

    for( i = 0; i < SB_QE2000_ELIB_NUM_CLASSES_K; i++ )
    {
        /*
         * Check for class enable changes
         */
        if( Cfg.bClassEna[i] != oldCfg.bClassEna[i] )
        {
            /*
             * This class has changed, we must transition
             * it accordingly
             */
            sZfPt.m_nCountTrans = i;
            (void) sbZfSbQe2000ElibPT_Pack( &sZfPt,
                                                    (uint8 *)aulData,
                                                    0 );

            
            nStatus = sbQe2000ElibClMemWrite( pEp->pHalCtx,
                                              SB_ZF_SB_QE2000_ELIB_PT_ENTRY_OFFSET + ulPort,
                                              aulData[0],
                                              aulData[1] );

            if( SB_ELIB_OK != nStatus )
            {
                DEXIT();
                return( nStatus );
            }

            switch(i)
            {
                case 1: sZfPt.m_bClassEnb1 = Cfg.bClassEna[1]; break;
                case 2: sZfPt.m_bClassEnb2 = Cfg.bClassEna[2]; break;
                case 3: sZfPt.m_bClassEnb3 = Cfg.bClassEna[3]; break;
                case 4: sZfPt.m_bClassEnb4 = Cfg.bClassEna[4]; break;
                case 5: sZfPt.m_bClassEnb5 = Cfg.bClassEna[5]; break;
                case 6: sZfPt.m_bClassEnb6 = Cfg.bClassEna[6]; break;
                case 7: sZfPt.m_bClassEnb7 = Cfg.bClassEna[7]; break;
                case 8: sZfPt.m_bClassEnb8 = Cfg.bClassEna[8]; break;
                case 9: sZfPt.m_bClassEnb9 = Cfg.bClassEna[9]; break;
                case 10: sZfPt.m_bClassEnb10 = Cfg.bClassEna[10]; break;
                case 11: sZfPt.m_bClassEnb11 = Cfg.bClassEna[11]; break;
                case 12: sZfPt.m_bClassEnb12 = Cfg.bClassEna[12]; break;
                case 13: sZfPt.m_bClassEnb13 = Cfg.bClassEna[13]; break;
                case 14: sZfPt.m_bClassEnb14 = Cfg.bClassEna[14]; break;
                case 15: sZfPt.m_bClassEnb15 = Cfg.bClassEna[15]; break;
                case 0:
                default:
                    sZfPt.m_bClassEnb0 = Cfg.bClassEna[0]; break;
            }

            (void) sbZfSbQe2000ElibPT_Pack( &sZfPt,
                                                    (uint8 *)aulData,
                                                    0 );

            
            nStatus = sbQe2000ElibClMemWrite( pEp->pHalCtx,
                                              SB_ZF_SB_QE2000_ELIB_PT_ENTRY_OFFSET + ulPort,
                                              aulData[0],
                                              aulData[1] );

            if( SB_ELIB_OK != nStatus )
            {
                DEXIT();
                return( nStatus );
            }

            /*
             * Turn off transition
             */
            sZfPt.m_nCountTrans = 63;
            (void) sbZfSbQe2000ElibPT_Pack( &sZfPt,
                                                    (uint8 *)aulData,
                                                    0 );

            nStatus = sbQe2000ElibClMemWrite( pEp->pHalCtx,
                                              SB_ZF_SB_QE2000_ELIB_PT_ENTRY_OFFSET + ulPort,
                                              aulData[0],
                                              aulData[1] );

            if( SB_ELIB_OK != nStatus )
            {
                DEXIT();
                return( nStatus );
            }

            /*
             * Read the CL transition counts
             */
            ulClTransitBytes = SAND_HAL_READ( pEp->pHalCtx,
                                             KA,
                                             EP_CL_TRANSITION_BYTE );
            ulClTransitPkts = SAND_HAL_READ( pEp->pHalCtx,
                                             KA,
                                             EP_CL_TRANSITION_PKT );
            switch(i)
            {
                case 1:
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_PktClass1, ulClTransitPkts);
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_ByteClass1, ulClTransitBytes);
                    break;
                case 2:
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_PktClass2, ulClTransitPkts);
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_ByteClass2, ulClTransitBytes);
                    break;
                case 3:
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_PktClass3, ulClTransitPkts);
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_ByteClass3, ulClTransitBytes);
                    break;
                case 4:
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_PktClass4, ulClTransitPkts);
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_ByteClass4, ulClTransitBytes);
                    break;
                case 5:
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_PktClass5, ulClTransitPkts);
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_ByteClass5, ulClTransitBytes);
                    break;
                case 6:
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_PktClass6, ulClTransitPkts);
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_ByteClass6, ulClTransitBytes);
                    break;
                case 7:
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_PktClass7, ulClTransitPkts);
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_ByteClass7, ulClTransitBytes);
                    break;
                case 8:
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_PktClass8, ulClTransitPkts);
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_ByteClass8, ulClTransitBytes);
                    break;
                case 9:
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_PktClass9, ulClTransitPkts);
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_ByteClass9, ulClTransitBytes);
                    break;
                case 10:
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_PktClass10, ulClTransitPkts);
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_ByteClass10, ulClTransitBytes);
                    break;
                case 11:
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_PktClass11, ulClTransitPkts);
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_ByteClass11, ulClTransitBytes);
                    break;
                case 12:
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_PktClass12, ulClTransitPkts);
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_ByteClass12, ulClTransitBytes);
                    break;
                case 13:
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_PktClass13, ulClTransitPkts);
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_ByteClass13, ulClTransitBytes);
                    break;
                case 14:
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_PktClass14, ulClTransitPkts);
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_ByteClass14, ulClTransitBytes);
                    break;
                case 15:
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_PktClass15, ulClTransitPkts);
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_ByteClass15, ulClTransitBytes);
                    break;
                case 0:
                default:
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_PktClass0, ulClTransitPkts);
                    COMPILER_64_ADD_32(pEp->asPCT[ulPort].m_ByteClass0, ulClTransitBytes);
                    break;
            }


        }

    }

    /*
     * Save the config to our local copy
     */
    pEp->asPortConfig[ulPort] = Cfg;
    DEXIT();

    return( SB_ELIB_OK );
}



sbElibStatus_et sbQe2000ElibPortCfgGet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulPort, SB_QE2000_ELIB_PORT_CFG_PST pCfg )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;

    DENTER();
    SB_ASSERT( Handle );
    if (ulPort >= SB_QE2000_ELIB_NUM_PORTS_K) {
        return (SB_ELIB_BAD_ARGS);
    }
    SB_ASSERT( pCfg );

    /*
     * Grab the cached copy
     */
    *pCfg = pEp->asPortConfig[ulPort];

    DEXIT();
    return( SB_ELIB_OK );
}



sbElibStatus_et sbQe2000ElibCRTBitSelectSet( SB_QE2000_ELIB_HANDLE Handle,
                                 SB_QE2000_ELIB_CRT_BIT_SEL_PST pBitSelect )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    int nHdrIdx;
    uint32 ulHdrSel;

    DENTER();
    SB_ASSERT( Handle );
    SB_ASSERT( pBitSelect );

    for( nHdrIdx = 0; nHdrIdx < SB_QE2000_ELIB_NUM_CRT_BITS_K; nHdrIdx++ )
    {
        switch( pBitSelect[nHdrIdx].SourceField )
        {
            case SB_QE2000_ELIB_CRT_BS_SRC_ERH:
                ulHdrSel = 0x7F & pBitSelect[nHdrIdx].ulOffset;
                break;

            case SB_QE2000_ELIB_CRT_BS_SRC_IS:
                ulHdrSel = 0x080 | ( 0x7F & pBitSelect[nHdrIdx].ulOffset );
                break;

            case SB_QE2000_ELIB_CRT_BS_SRC_PAYLOAD:
                ulHdrSel = 0x200 | (0x1FF & pBitSelect[nHdrIdx].ulOffset );
                break;

            default:
                DEXIT();
                return( SB_ELIB_BAD_IDX );
                break;
        }

        /*
         * Write the new header selection
         */
        SAND_HAL_WRITE_INDEX( pEp->pHalCtx,
                              KA,
                              EP_CL_HDR_SEL0,
                              nHdrIdx,
                              ulHdrSel );
    } /* end for each hdr */

    DEXIT();
    return( SB_ELIB_OK );
}

sbElibStatus_et sbQe2000ElibCRTBitSelectGet( SB_QE2000_ELIB_HANDLE Handle,
                                 SB_QE2000_ELIB_CRT_BIT_SEL_PST pBitSelect )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    int nHdrIdx;
    uint32 ulHdrSel;

    DENTER();
    SB_ASSERT( Handle );
    SB_ASSERT( pBitSelect );

    for( nHdrIdx = 0; nHdrIdx < SB_QE2000_ELIB_NUM_CRT_BITS_K; nHdrIdx++ )
    {
        /*
         * Retrieve the current header selection
         */
        ulHdrSel = SAND_HAL_READ_INDEX( pEp->pHalCtx,
                                        KA,
                                        EP_CL_HDR_SEL0,
                                        nHdrIdx );

        /*
         * Parse the fields
         */
        if( 0x200 & ulHdrSel ) /* Source is Payload */
        {
            pBitSelect[nHdrIdx].SourceField = SB_QE2000_ELIB_CRT_BS_SRC_PAYLOAD;
            pBitSelect[nHdrIdx].ulOffset = 0x1FF & ulHdrSel;
        }
        else if( 0x080 & ulHdrSel ) /* Source is Instruction Stack */
        {
            pBitSelect[nHdrIdx].SourceField = SB_QE2000_ELIB_CRT_BS_SRC_IS;
            pBitSelect[nHdrIdx].ulOffset = 0x7F & ulHdrSel;
        }
        else /* Must be ERH */
        {
            pBitSelect[nHdrIdx].SourceField = SB_QE2000_ELIB_CRT_BS_SRC_ERH;
            pBitSelect[nHdrIdx].ulOffset = 0x7F & ulHdrSel;
        }

    } /* end for each hdr */

    DEXIT();
    return( SB_ELIB_OK );
}

sbElibStatus_et sbQe2000ElibCRTSet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulIndex, uint32 ulClass )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    uint32 ulWordIdx, ulEntry;

    DENTER();
    SB_ASSERT( Handle );
    SB_ASSERT( ulIndex < SB_QE2000_ELIB_NUM_CRT_ENTRIES_K );
    SB_ASSERT( ulClass < SB_QE2000_ELIB_NUM_CLASSES_K );

    if (ulIndex >= SB_QE2000_ELIB_NUM_CRT_ENTRIES_K) {
        return (SB_ELIB_BAD_ARGS);
    }

    if (ulClass >= SB_QE2000_ELIB_NUM_CLASSES_K) {
        return (SB_ELIB_BAD_ARGS);
    }

    /*
     * Get the index into the CRT
     */
    ulWordIdx = ulIndex / SB_QE2000_ELIB_NUM_CLASSES_K;

    /*
     * Get the entry in the index
     */
    ulEntry = ulIndex % SB_QE2000_ELIB_NUM_CLASSES_K;

    /*
     * Save off the CRT in our private stash
     */
    switch( ulEntry )
    {
        case 0:
            pEp->asCRT[ulWordIdx].m_nClass0 = ulClass;
            break;

        case 1:
            pEp->asCRT[ulWordIdx].m_nClass1 = ulClass;
            break;

        case 2:
            pEp->asCRT[ulWordIdx].m_nClass2 = ulClass;
            break;

        case 3:
            pEp->asCRT[ulWordIdx].m_nClass3 = ulClass;
            break;

        case 4:
            pEp->asCRT[ulWordIdx].m_nClass4 = ulClass;
            break;

        case 5:
            pEp->asCRT[ulWordIdx].m_nClass5 = ulClass;
            break;

        case 6:
            pEp->asCRT[ulWordIdx].m_nClass6 = ulClass;
            break;

        case 7:
            pEp->asCRT[ulWordIdx].m_nClass7 = ulClass;
            break;

        case 8:
            pEp->asCRT[ulWordIdx].m_nClass8 = ulClass;
            break;

        case 9:
            pEp->asCRT[ulWordIdx].m_nClass9 = ulClass;
            break;

        case 10:
            pEp->asCRT[ulWordIdx].m_nClass10 = ulClass;
            break;

        case 11:
            pEp->asCRT[ulWordIdx].m_nClass11 = ulClass;
            break;

        case 12:
            pEp->asCRT[ulWordIdx].m_nClass12 = ulClass;
            break;

        case 13:
            pEp->asCRT[ulWordIdx].m_nClass13 = ulClass;
            break;

        case 14:
            pEp->asCRT[ulWordIdx].m_nClass14 = ulClass;
            break;

        case 15:
            pEp->asCRT[ulWordIdx].m_nClass15 = ulClass;
            break;
        /* coverity[dead_error_begin] */
        default:
            break;
    } /* end switch ulEntry */

    pEp->abCRTDirty[ulWordIdx] = TRUE;

    DEXIT();
    return( SB_ELIB_OK );
}

sbElibStatus_et sbQe2000ElibCRTPush( SB_QE2000_ELIB_HANDLE Handle, bool_t bAll )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    sbElibStatus_et nStatus;
    int nWordIdx;
    uint32 aulCRT[SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_SIZE_IN_BYTES / sizeof(uint32)];

    DENTER();

    SB_ASSERT( Handle );


    for( nWordIdx = 0; nWordIdx < SB_QE2000_ELIB_NUM_CRT_ENTRIES_K / SB_QE2000_ELIB_NUM_CLASSES_K; nWordIdx++ )
    {
        if( (TRUE == pEp->abCRTDirty[nWordIdx]) || (TRUE == bAll) )
        {
            sbZfSbQe2000ElibCRT_Pack( &pEp->asCRT[nWordIdx],
                                              (uint8 *)aulCRT,
                                              SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_SIZE_IN_BYTES );

            nStatus = sbQe2000ElibClMemWrite( pEp->pHalCtx,
                                              ( SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_OFFSET + nWordIdx ),
                                              aulCRT[0],
                                              aulCRT[1] );
            if( SB_ELIB_OK != nStatus )
            {
                DEXIT();
                return( nStatus );
            }

            pEp->abCRTDirty[nWordIdx] = FALSE;
        }
    }

    DEXIT();
    return( SB_ELIB_OK );

}


sbElibStatus_et sbQe2000ElibCRTGet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulIndex, uint32* pulClass )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    sbElibStatus_et status;
    uint32 ulWordIdx, ulEntry;

    DENTER();
    SB_ASSERT( Handle );
    if (ulIndex >= SB_QE2000_ELIB_NUM_CRT_ENTRIES_K) {
        return (SB_ELIB_BAD_ARGS);
    }

    SB_ASSERT( pulClass );

    /*
     * Get the index into the CRT
     */
    ulWordIdx = ulIndex / SB_QE2000_ELIB_NUM_CLASSES_K;

    /*
     * Get the entry in the index
     */
    ulEntry = ulIndex % SB_QE2000_ELIB_NUM_CLASSES_K;

    status = SB_ELIB_OK;

    switch( ulEntry )
    {
        case 0:
            *pulClass = pEp->asCRT[ulWordIdx].m_nClass0;
            break;

        case 1:
            *pulClass = pEp->asCRT[ulWordIdx].m_nClass1;
            break;

        case 2:
            *pulClass = pEp->asCRT[ulWordIdx].m_nClass2;
            break;

        case 3:
            *pulClass = pEp->asCRT[ulWordIdx].m_nClass3;
            break;

        case 4:
            *pulClass = pEp->asCRT[ulWordIdx].m_nClass4;
            break;

        case 5:
            *pulClass = pEp->asCRT[ulWordIdx].m_nClass5;
            break;

        case 6:
            *pulClass = pEp->asCRT[ulWordIdx].m_nClass6;
            break;

        case 7:
            *pulClass = pEp->asCRT[ulWordIdx].m_nClass7;
            break;

        case 8:
            *pulClass = pEp->asCRT[ulWordIdx].m_nClass8;
            break;

        case 9:
            *pulClass = pEp->asCRT[ulWordIdx].m_nClass9;
            break;

        case 10:
            *pulClass = pEp->asCRT[ulWordIdx].m_nClass10;
            break;

        case 11:
            *pulClass = pEp->asCRT[ulWordIdx].m_nClass11;
            break;

        case 12:
            *pulClass = pEp->asCRT[ulWordIdx].m_nClass12;
            break;

        case 13:
            *pulClass = pEp->asCRT[ulWordIdx].m_nClass13;
            break;

        case 14:
            *pulClass = pEp->asCRT[ulWordIdx].m_nClass14;
            break;

        case 15:
            *pulClass = pEp->asCRT[ulWordIdx].m_nClass15;
            break;

        /* coverity[dead_error_begin] */
        default:
            *pulClass = 0;
            status = SB_ELIB_BAD_IDX;
            break;
    } /* end switch ulEntry */

    DEXIT();
    return( status );
}

sbElibStatus_et sbQe2000ElibCITSet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulClass, SB_QE2000_ELIB_CIT_ENTRY_ST Inst )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    uint32 aulInst[SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_SIZE_IN_BYTES / sizeof (uint32)];
    sbElibStatus_et status;
    int nInstr;
    uint32 ulClassOffset;
    uint32 ulAppend;
    uint8 ucAppend;

    DENTER();
    SB_ASSERT( Handle );
    SB_ASSERT( ulClass < SB_QE2000_ELIB_NUM_CLASSES_K );

    switch( ulClass )
    {
        case 0:
        case 1:
        case 2:
        case 3:
            ulAppend = SAND_HAL_READ(pEp->pHalCtx, KA, EP_CL_INST_CTRL0);
            break;

        case 4:
        case 5:
        case 6:
        case 7:
            ulAppend = SAND_HAL_READ(pEp->pHalCtx, KA, EP_CL_INST_CTRL1);
            break;

        case 8:
        case 9:
        case 10:
        case 11:
            ulAppend = SAND_HAL_READ(pEp->pHalCtx, KA, EP_CL_INST_CTRL2);
            break;

        case 12:
        case 13:
        case 14:
        case 15:
            ulAppend = SAND_HAL_READ(pEp->pHalCtx, KA, EP_CL_INST_CTRL3);
            break;

        default:
            DEXIT();
            return( SB_ELIB_BAD_IDX );
            break;
    }


    ucAppend = 0;

    for( nInstr = 0; nInstr < SB_QE2000_ELIB_NUM_CLASS_INSTR_K; nInstr++)
    {
        if( TRUE == Inst.bAppend[nInstr] )
        {
            ucAppend |= 0x1 << nInstr;
        }
    }

    switch( ulClass )
    {
        case 0:
        case 4:
        case 8:
        case 12:
            ulAppend = SAND_HAL_MOD_FIELD( KA, EP_CL_INST_CTRL0,
                                           CLASS0, ulAppend, ucAppend );
            break;

        case 1:
        case 5:
    /*    coverity[equality_cond]    */
        case 9:
        case 13:
            ulAppend = SAND_HAL_MOD_FIELD( KA, EP_CL_INST_CTRL0,
                                           CLASS1, ulAppend, ucAppend );
            break;

        case 2:
        case 6:
        case 10:
        case 14:
            ulAppend = SAND_HAL_MOD_FIELD( KA, EP_CL_INST_CTRL0,
                                           CLASS2, ulAppend, ucAppend );
            break;

        case 3:
        case 7:
        case 11:
        case 15:
            ulAppend = SAND_HAL_MOD_FIELD( KA, EP_CL_INST_CTRL0,
                                           CLASS3, ulAppend, ucAppend );
            break;

        default:
            DEXIT();
            return( -6 );
            break;
    }


    if( TRUE == Inst.bInstValid[0] )
    {
        pEp->asCIT[ulClass].m_Instruction0 =
            ( 0x80000000 | Inst.ulInst[0] );
    }
    else
    {
        pEp->asCIT[ulClass].m_Instruction0 =
            ( ~0x80000000 & Inst.ulInst[0] );
    }

    if( TRUE == Inst.bInstValid[1] )
    {
        pEp->asCIT[ulClass].m_Instruction1 =
            ( 0x80000000 | Inst.ulInst[1] );
    }
    else
    {
        pEp->asCIT[ulClass].m_Instruction1 =
            ( ~0x80000000 & Inst.ulInst[1] );
    }

    if( TRUE == Inst.bInstValid[2] )
    {
        pEp->asCIT[ulClass].m_Instruction2 =
            ( 0x80000000 | Inst.ulInst[2] );
    }
    else
    {
        pEp->asCIT[ulClass].m_Instruction2 =
            ( ~0x80000000 & Inst.ulInst[2] );
    }

    if( TRUE == Inst.bInstValid[3] )
    {
        pEp->asCIT[ulClass].m_Instruction3 =
            ( 0x80000000 | Inst.ulInst[3] );
    }
    else
    {
        pEp->asCIT[ulClass].m_Instruction3 =
            ( ~0x80000000 & Inst.ulInst[3] );
    }

    if( TRUE == Inst.bInstValid[4] )
    {
        pEp->asCIT[ulClass].m_Instruction4 =
            ( 0x80000000 | Inst.ulInst[4] );
    }
    else
    {
        pEp->asCIT[ulClass].m_Instruction4 =
            ( ~0x80000000 & Inst.ulInst[4] );
    }

    if( TRUE == Inst.bInstValid[5] )
    {
        pEp->asCIT[ulClass].m_Instruction5 =
            ( 0x80000000 | Inst.ulInst[5] );
    }
    else
    {
        pEp->asCIT[ulClass].m_Instruction5 =
            ( ~0x80000000 & Inst.ulInst[5] );
    }

    if( TRUE == Inst.bInstValid[6] )
    {
        pEp->asCIT[ulClass].m_Instruction6 =
            ( 0x80000000 | Inst.ulInst[6] );
    }
    else
    {
        pEp->asCIT[ulClass].m_Instruction6 =
            ( ~0x80000000 & Inst.ulInst[6] );
    }

    if( TRUE == Inst.bInstValid[7] )
    {
        pEp->asCIT[ulClass].m_Instruction7 =
            ( 0x80000000 | Inst.ulInst[7] );
    }
    else
    {
        pEp->asCIT[ulClass].m_Instruction7 =
            ( ~0x80000000 & Inst.ulInst[7] );
    }

    /*
     * Pack it up
     */
    (void)sbZfSbQe2000ElibCIT_Pack( &pEp->asCIT[ulClass],
                                            (uint8 *)aulInst,
                                            SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_SIZE );

    ulClassOffset = (ulClass * 4) + SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_OFFSET;

    status = sbQe2000ElibClMemWrite( pEp->pHalCtx,
                                     ulClassOffset,
                                     aulInst[0],
                                     aulInst[1] );

    if( SB_ELIB_OK != status )
    {
        DEXIT();
        return( status );
    }

    status = sbQe2000ElibClMemWrite( pEp->pHalCtx,
                                     ulClassOffset + 1,
                                     aulInst[2],
                                     aulInst[3] );

    if( SB_ELIB_OK != status )
    {
        DEXIT();
        return( status );
    }

    status = sbQe2000ElibClMemWrite( pEp->pHalCtx,
                                     ulClassOffset + 2,
                                     aulInst[4],
                                     aulInst[5] );

    if( SB_ELIB_OK != status )
    {
        DEXIT();
        return( status );
    }

    status = sbQe2000ElibClMemWrite( pEp->pHalCtx,
                                     ulClassOffset + 3,
                                     aulInst[6],
                                     aulInst[7] );

    if( SB_ELIB_OK != status )
    {
        DEXIT();
        return( status );
    }

    /*
     * Set the [pre|ap]pend bits
     */
    switch( ulClass )
    {
        case 0:
        case 1:
        case 2:
        case 3:
            SAND_HAL_WRITE(pEp->pHalCtx, KA, EP_CL_INST_CTRL0, ulAppend);
            break;

        case 4:
        case 5:
        case 6:
        case 7:
            SAND_HAL_WRITE(pEp->pHalCtx, KA, EP_CL_INST_CTRL1, ulAppend);
            break;

        case 8:
        case 9:
        case 10:
        case 11:
            SAND_HAL_WRITE(pEp->pHalCtx, KA, EP_CL_INST_CTRL2, ulAppend);
            break;

        case 12:
        case 13:
        case 14:
        case 15:
            SAND_HAL_WRITE(pEp->pHalCtx, KA, EP_CL_INST_CTRL3, ulAppend);
            break;

        /* Defensive Default */
        /* coverity[dead_error_begin] */
        default:
            DEXIT();
            return( SB_ELIB_BAD_IDX );
            break;
    }

    DEXIT();
    return( SB_ELIB_OK );
}

sbElibStatus_et sbQe2000ElibCITGet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulClass, SB_QE2000_ELIB_CIT_ENTRY_PST pInst )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    uint32 aulInst[SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_SIZE_IN_BYTES / sizeof (uint32)];
    sbElibStatus_et status;
    int nInstr;
    uint32 ulClassOffset;
    uint32 ulAppend;

    DENTER();
    SB_ASSERT( Handle );
    SB_ASSERT( ulClass < SB_QE2000_ELIB_NUM_CLASSES_K );
    SB_ASSERT( pInst );


    switch( ulClass )
    {
        case 0:
        case 1:
        case 2:
        case 3:
            ulAppend = SAND_HAL_READ(pEp->pHalCtx, KA, EP_CL_INST_CTRL0);
            break;

        case 4:
        case 5:
        case 6:
        case 7:
            ulAppend = SAND_HAL_READ(pEp->pHalCtx, KA, EP_CL_INST_CTRL1);
            break;

    /*    coverity[equality_cond]    */
        case 8:
        case 9:
        case 10:
        case 11:
            ulAppend = SAND_HAL_READ(pEp->pHalCtx, KA, EP_CL_INST_CTRL2);
            break;

        case 12:
        case 13:
        case 14:
        case 15:
            ulAppend = SAND_HAL_READ(pEp->pHalCtx, KA, EP_CL_INST_CTRL3);
            break;

        default:
            DEXIT();
            return( SB_ELIB_BAD_IDX );
            break;
    }

    switch( ulClass )
    {
        case 0:
        case 4:
        case 8:
        case 12:
            ulAppend = SAND_HAL_GET_FIELD( KA, EP_CL_INST_CTRL0,
                                           CLASS0, ulAppend );
            break;

        case 1:
        case 5:
        case 9:
        case 13:
            ulAppend = SAND_HAL_GET_FIELD( KA, EP_CL_INST_CTRL0,
                                           CLASS1, ulAppend );
            break;

        case 2:
        case 6:
        case 10:
        case 14:
            ulAppend = SAND_HAL_GET_FIELD( KA, EP_CL_INST_CTRL0,
                                           CLASS2, ulAppend );
            break;

        case 3:
        case 7:
        case 11:
        case 15:
            ulAppend = SAND_HAL_GET_FIELD( KA, EP_CL_INST_CTRL0,
                                           CLASS3, ulAppend );
            break;

        /* Defensive Default */
        /* coverity[dead_error_begin] */
        default:
            DEXIT();
            return( SB_ELIB_BAD_IDX );
            break;
    }


    for( nInstr = 0; nInstr < SB_QE2000_ELIB_NUM_CLASS_INSTR_K; nInstr++)
    {
        if( (0x1 << nInstr) & ulAppend )
        {
            pInst->bAppend[nInstr] = TRUE;
        }
        else
        {
            pInst->bAppend[nInstr] = FALSE;
        }
    }

    ulClassOffset = (ulClass * 4) + SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_OFFSET;

    status = sbQe2000ElibClMemRead( pEp->pHalCtx,
                                    ulClassOffset,
                                    0,
                                    &aulInst[0],
                                    &aulInst[1] );

    if( SB_ELIB_OK != status )
    {
        DEXIT();
        return( status );
    }

    status = sbQe2000ElibClMemRead( pEp->pHalCtx,
                                    ulClassOffset + 1,
                                    0,
                                    &aulInst[2],
                                    &aulInst[3] );

    if( SB_ELIB_OK != status )
    {
        DEXIT();
        return( status );
    }

    status = sbQe2000ElibClMemRead( pEp->pHalCtx,
                                    ulClassOffset + 2,
                                    0,
                                    &aulInst[4],
                                    &aulInst[5] );

    if( SB_ELIB_OK != status )
    {
        DEXIT();
        return( status );
    }

    status = sbQe2000ElibClMemRead( pEp->pHalCtx,
                                    ulClassOffset + 3,
                                    0,
                                    &aulInst[6],
                                    &aulInst[7] );

    if( SB_ELIB_OK != status )
    {
        DEXIT();
        return( status );
    }

    /*
     * We have the data, now unpack it
     */
    sbZfSbQe2000ElibCIT_Unpack( &pEp->asCIT[ulClass],
                                        (uint8*)aulInst,
                                        SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_SIZE );

    /*
     * Everything is unpacked, now fill in the user's structure
     */

    /*
     * Instruction 0
     */
    if( 0x80000000 & pEp->asCIT[ulClass].m_Instruction0 )
    {
        pInst->bInstValid[0] = TRUE;
    }
    else
    {
        pInst->bInstValid[0] = FALSE;
    }

    pInst->ulInst[0] = ( ~0x80000000 & pEp->asCIT[ulClass].m_Instruction0 );

    /*
     * Instruction 1
     */
    if( 0x80000000 & pEp->asCIT[ulClass].m_Instruction1 )
    {
        pInst->bInstValid[1] = TRUE;
    }
    else
    {
        pInst->bInstValid[1] = FALSE;
    }

    pInst->ulInst[1] = ( ~0x80000000 & pEp->asCIT[ulClass].m_Instruction1 );

    /*
     * Instruction 2
     */
    if( 0x80000000 & pEp->asCIT[ulClass].m_Instruction2 )
    {
        pInst->bInstValid[2] = TRUE;
    }
    else
    {
        pInst->bInstValid[2] = FALSE;
    }

    pInst->ulInst[2] = ( ~0x80000000 & pEp->asCIT[ulClass].m_Instruction2 );

    /*
     * Instruction 3
     */
    if( 0x80000000 & pEp->asCIT[ulClass].m_Instruction3 )
    {
        pInst->bInstValid[3] = TRUE;
    }
    else
    {
        pInst->bInstValid[3] = FALSE;
    }

    pInst->ulInst[3] = ( ~0x80000000 & pEp->asCIT[ulClass].m_Instruction3 );

    /*
     * Instruction 4
     */
    if( 0x80000000 & pEp->asCIT[ulClass].m_Instruction4 )
    {
        pInst->bInstValid[4] = TRUE;
    }
    else
    {
        pInst->bInstValid[4] = FALSE;
    }

    pInst->ulInst[4] = ( ~0x80000000 & pEp->asCIT[ulClass].m_Instruction4 );

    /*
     * Instruction 5
     */
    if( 0x80000000 & pEp->asCIT[ulClass].m_Instruction5 )
    {
        pInst->bInstValid[5] = TRUE;
    }
    else
    {
        pInst->bInstValid[5] = FALSE;
    }

    pInst->ulInst[5] = ( ~0x80000000 & pEp->asCIT[ulClass].m_Instruction5 );

    /*
     * Instruction 6
     */
    if( 0x80000000 & pEp->asCIT[ulClass].m_Instruction6 )
    {
        pInst->bInstValid[6] = TRUE;
    }
    else
    {
        pInst->bInstValid[6] = FALSE;
    }

    pInst->ulInst[6] = ( ~0x80000000 & pEp->asCIT[ulClass].m_Instruction6 );

    /*
     * Instruction 7
     */
    if( 0x80000000 & pEp->asCIT[ulClass].m_Instruction7 )
    {
        pInst->bInstValid[7] = TRUE;
    }
    else
    {
        pInst->bInstValid[7] = FALSE;
    }

    pInst->ulInst[7] = ( ~0x80000000 & pEp->asCIT[ulClass].m_Instruction7 );

    DEXIT();
    return( SB_ELIB_OK );
}


sbElibStatus_et sbQe2000ElibPCTGet( SB_QE2000_ELIB_HANDLE Handle,
                        uint32 ulPort,
                        uint32 ulClass,
                        uint64 *pullPacketCnt,
                        uint64 *pullByteCnt )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    sbElibStatus_et nStatus;

    DENTER();
    SB_ASSERT( Handle );
    SB_ASSERT( ulPort < SB_QE2000_ELIB_NUM_PORTS_K );
    SB_ASSERT( ulClass < SB_QE2000_ELIB_NUM_CLASSES_K );
    SB_ASSERT( pullPacketCnt );
    SB_ASSERT( pullByteCnt );


    nStatus = sbQe2000ElibSemGet(pEp, SB_QE2000_ELIB_SEM_TO_K);
    if( 0 != nStatus )
    {
        return( nStatus );
    }

    switch(ulClass)
    {
        case 0:
            *pullPacketCnt = pEp->asPCT[ulPort].m_PktClass0;
            *pullByteCnt = pEp->asPCT[ulPort].m_ByteClass0;
            break;
        case 1:
            *pullPacketCnt = pEp->asPCT[ulPort].m_PktClass1;
            *pullByteCnt = pEp->asPCT[ulPort].m_ByteClass1;
            break;
        case 2:
            *pullPacketCnt = pEp->asPCT[ulPort].m_PktClass2;
            *pullByteCnt = pEp->asPCT[ulPort].m_ByteClass2;
            break;
        case 3:
            *pullPacketCnt = pEp->asPCT[ulPort].m_PktClass3;
            *pullByteCnt = pEp->asPCT[ulPort].m_ByteClass3;
            break;
        case 4:
            *pullPacketCnt = pEp->asPCT[ulPort].m_PktClass4;
            *pullByteCnt = pEp->asPCT[ulPort].m_ByteClass4;
            break;
        case 5:
            *pullPacketCnt = pEp->asPCT[ulPort].m_PktClass5;
            *pullByteCnt = pEp->asPCT[ulPort].m_ByteClass5;
            break;
        case 6:
            *pullPacketCnt = pEp->asPCT[ulPort].m_PktClass6;
            *pullByteCnt = pEp->asPCT[ulPort].m_ByteClass6;
            break;
        case 7:
            *pullPacketCnt = pEp->asPCT[ulPort].m_PktClass7;
            *pullByteCnt = pEp->asPCT[ulPort].m_ByteClass7;
            break;
        case 8:
            *pullPacketCnt = pEp->asPCT[ulPort].m_PktClass8;
            *pullByteCnt = pEp->asPCT[ulPort].m_ByteClass8;
            break;
        case 9:
            *pullPacketCnt = pEp->asPCT[ulPort].m_PktClass9;
            *pullByteCnt = pEp->asPCT[ulPort].m_ByteClass9;
            break;
        case 10:
            *pullPacketCnt = pEp->asPCT[ulPort].m_PktClass10;
            *pullByteCnt = pEp->asPCT[ulPort].m_ByteClass10;
            break;
        case 11:
            *pullPacketCnt = pEp->asPCT[ulPort].m_PktClass11;
            *pullByteCnt = pEp->asPCT[ulPort].m_ByteClass11;
            break;
        case 12:
            *pullPacketCnt = pEp->asPCT[ulPort].m_PktClass12;
            *pullByteCnt = pEp->asPCT[ulPort].m_ByteClass12;
            break;
        case 13:
            *pullPacketCnt = pEp->asPCT[ulPort].m_PktClass13;
            *pullByteCnt = pEp->asPCT[ulPort].m_ByteClass13;
            break;
        case 14:
            *pullPacketCnt = pEp->asPCT[ulPort].m_PktClass14;
            *pullByteCnt = pEp->asPCT[ulPort].m_ByteClass14;
            break;
        case 15:
            *pullPacketCnt = pEp->asPCT[ulPort].m_PktClass15;
            *pullByteCnt = pEp->asPCT[ulPort].m_ByteClass15;
            break;
        default:
            sbQe2000ElibSemGive(pEp);
           return( SB_ELIB_BAD_IDX );
            break;
    }

    sbQe2000ElibSemGive(pEp);

    DEXIT();
    return( SB_ELIB_OK );
}



sbElibStatus_et sbQe2000ElibCmapSet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulSelect, SB_QE2000_ELIB_CMAP_ST Map )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    uint32 ulCmapFwd0, ulCmapFwd1;
    uint32 ulCmapDrp0, ulCmapDrp1;
    uint32 ulBfConfigReg;
    int anCmapFwdCnt[SB_QE2000_ELIB_NUM_CLASSES_K];
    int anCmapDrpCnt[SB_QE2000_ELIB_NUM_CLASSES_K];
    int nCmapFwdCnt;
    int nCmapDrpCnt;
    int i;

    DENTER();
    SB_ASSERT( Handle );
    SB_ASSERT( ulSelect < SB_QE2000_ELIB_NUM_CMAPS_K );

    nCmapFwdCnt = 0;
    nCmapDrpCnt = 0;

    for( i = 0; i < SB_QE2000_ELIB_NUM_CLASSES_K; i++ )
    {
        anCmapFwdCnt[i] = 0;
        anCmapDrpCnt[i] = 0;
    }

    for( i = 0; i < SB_QE2000_ELIB_NUM_CLASSES_K; i++ )
    {
        anCmapFwdCnt[Map.ulFwdMap[i]] = 1;
        anCmapDrpCnt[Map.ulDrpMap[i]] = 1;
    }

    for( i = 0; i < SB_QE2000_ELIB_NUM_CLASSES_K; i++ )
    {
        nCmapFwdCnt += anCmapFwdCnt[i];
        nCmapDrpCnt += anCmapDrpCnt[i];
    }

    ulCmapFwd0  = SAND_HAL_SET_FIELD( KA, EP_BF_FWD_CLASS_MAP0_0, CNT0,  Map.ulFwdMap[0] );
    ulCmapFwd0 |= SAND_HAL_SET_FIELD( KA, EP_BF_FWD_CLASS_MAP0_0, CNT1,  Map.ulFwdMap[1] );
    ulCmapFwd0 |= SAND_HAL_SET_FIELD( KA, EP_BF_FWD_CLASS_MAP0_0, CNT2,  Map.ulFwdMap[2] );
    ulCmapFwd0 |= SAND_HAL_SET_FIELD( KA, EP_BF_FWD_CLASS_MAP0_0, CNT3,  Map.ulFwdMap[3] );
    ulCmapFwd0 |= SAND_HAL_SET_FIELD( KA, EP_BF_FWD_CLASS_MAP0_0, CNT4,  Map.ulFwdMap[4] );
    ulCmapFwd0 |= SAND_HAL_SET_FIELD( KA, EP_BF_FWD_CLASS_MAP0_0, CNT5,  Map.ulFwdMap[5] );
    ulCmapFwd0 |= SAND_HAL_SET_FIELD( KA, EP_BF_FWD_CLASS_MAP0_0, CNT6,  Map.ulFwdMap[6] );
    ulCmapFwd0 |= SAND_HAL_SET_FIELD( KA, EP_BF_FWD_CLASS_MAP0_0, CNT7,  Map.ulFwdMap[7] );

    ulCmapFwd1  = SAND_HAL_SET_FIELD( KA, EP_BF_FWD_CLASS_MAP0_1, CNT8,  Map.ulFwdMap[8] );
    ulCmapFwd1 |= SAND_HAL_SET_FIELD( KA, EP_BF_FWD_CLASS_MAP0_1, CNT9,  Map.ulFwdMap[9] );
    ulCmapFwd1 |= SAND_HAL_SET_FIELD( KA, EP_BF_FWD_CLASS_MAP0_1, CNT10, Map.ulFwdMap[10] );
    ulCmapFwd1 |= SAND_HAL_SET_FIELD( KA, EP_BF_FWD_CLASS_MAP0_1, CNT11, Map.ulFwdMap[11] );
    ulCmapFwd1 |= SAND_HAL_SET_FIELD( KA, EP_BF_FWD_CLASS_MAP0_1, CNT12, Map.ulFwdMap[12] );
    ulCmapFwd1 |= SAND_HAL_SET_FIELD( KA, EP_BF_FWD_CLASS_MAP0_1, CNT13, Map.ulFwdMap[13] );
    ulCmapFwd1 |= SAND_HAL_SET_FIELD( KA, EP_BF_FWD_CLASS_MAP0_1, CNT14, Map.ulFwdMap[14] );
    ulCmapFwd1 |= SAND_HAL_SET_FIELD( KA, EP_BF_FWD_CLASS_MAP0_1, CNT15, Map.ulFwdMap[15] );

    ulCmapDrp0  = SAND_HAL_SET_FIELD( KA, EP_BF_DROP_CLASS_MAP0_0, CNT0, Map.ulDrpMap[0] );
    ulCmapDrp0 |= SAND_HAL_SET_FIELD( KA, EP_BF_DROP_CLASS_MAP0_0, CNT1, Map.ulDrpMap[1] );
    ulCmapDrp0 |= SAND_HAL_SET_FIELD( KA, EP_BF_DROP_CLASS_MAP0_0, CNT2, Map.ulDrpMap[2] );
    ulCmapDrp0 |= SAND_HAL_SET_FIELD( KA, EP_BF_DROP_CLASS_MAP0_0, CNT3, Map.ulDrpMap[3] );
    ulCmapDrp0 |= SAND_HAL_SET_FIELD( KA, EP_BF_DROP_CLASS_MAP0_0, CNT4, Map.ulDrpMap[4] );
    ulCmapDrp0 |= SAND_HAL_SET_FIELD( KA, EP_BF_DROP_CLASS_MAP0_0, CNT5, Map.ulDrpMap[5] );
    ulCmapDrp0 |= SAND_HAL_SET_FIELD( KA, EP_BF_DROP_CLASS_MAP0_0, CNT6, Map.ulDrpMap[6] );
    ulCmapDrp0 |= SAND_HAL_SET_FIELD( KA, EP_BF_DROP_CLASS_MAP0_0, CNT7, Map.ulDrpMap[7] );

    ulCmapDrp1  = SAND_HAL_SET_FIELD( KA, EP_BF_DROP_CLASS_MAP0_1, CNT8,  Map.ulDrpMap[8] );
    ulCmapDrp1 |= SAND_HAL_SET_FIELD( KA, EP_BF_DROP_CLASS_MAP0_1, CNT9,  Map.ulDrpMap[9] );
    ulCmapDrp1 |= SAND_HAL_SET_FIELD( KA, EP_BF_DROP_CLASS_MAP0_1, CNT10, Map.ulDrpMap[10] );
    ulCmapDrp1 |= SAND_HAL_SET_FIELD( KA, EP_BF_DROP_CLASS_MAP0_1, CNT11, Map.ulDrpMap[11] );
    ulCmapDrp1 |= SAND_HAL_SET_FIELD( KA, EP_BF_DROP_CLASS_MAP0_1, CNT12, Map.ulDrpMap[12] );
    ulCmapDrp1 |= SAND_HAL_SET_FIELD( KA, EP_BF_DROP_CLASS_MAP0_1, CNT13, Map.ulDrpMap[13] );
    ulCmapDrp1 |= SAND_HAL_SET_FIELD( KA, EP_BF_DROP_CLASS_MAP0_1, CNT14, Map.ulDrpMap[14] );
    ulCmapDrp1 |= SAND_HAL_SET_FIELD( KA, EP_BF_DROP_CLASS_MAP0_1, CNT15, Map.ulDrpMap[15] );

    pEp->nCMapClassCnt[ulSelect] = nCmapFwdCnt + nCmapDrpCnt;

    ulBfConfigReg = SAND_HAL_READ( pEp->pHalCtx, KA, EP_BF_CONFIG);
    if( 0 == ulSelect )
    {
        SAND_HAL_WRITE( pEp->pHalCtx, KA, EP_BF_FWD_CLASS_MAP0_0, ulCmapFwd0 );
        SAND_HAL_WRITE( pEp->pHalCtx, KA, EP_BF_FWD_CLASS_MAP0_1, ulCmapFwd1 );

        SAND_HAL_WRITE( pEp->pHalCtx, KA, EP_BF_DROP_CLASS_MAP0_0, ulCmapDrp0 );
        SAND_HAL_WRITE( pEp->pHalCtx, KA, EP_BF_DROP_CLASS_MAP0_1, ulCmapDrp1 );

        ulBfConfigReg = SAND_HAL_MOD_FIELD(KA, EP_BF_CONFIG,
                                           MAP0_COUNTERS,
                                           ulBfConfigReg,
                                           pEp->nCMapClassCnt[0]);
    }
    else
    {
        SAND_HAL_WRITE( pEp->pHalCtx, KA, EP_BF_FWD_CLASS_MAP1_0, ulCmapFwd0 );
        SAND_HAL_WRITE( pEp->pHalCtx, KA, EP_BF_FWD_CLASS_MAP1_1, ulCmapFwd1 );

        SAND_HAL_WRITE( pEp->pHalCtx, KA, EP_BF_DROP_CLASS_MAP1_0, ulCmapDrp0 );
        SAND_HAL_WRITE( pEp->pHalCtx, KA, EP_BF_DROP_CLASS_MAP1_1, ulCmapDrp1 );

        ulBfConfigReg = SAND_HAL_MOD_FIELD(KA, EP_BF_CONFIG,
                                           MAP1_COUNTERS,
                                           ulBfConfigReg,
                                           pEp->nCMapClassCnt[1]);
    }

    SAND_HAL_WRITE( pEp->pHalCtx, KA, EP_BF_CONFIG, ulBfConfigReg);

    /*
     * Save off the class map for future use
     */

    pEp->sCmap[ulSelect] = Map;

    DEXIT();
    return( SB_ELIB_OK );
}

sbElibStatus_et sbQe2000ElibCmapGet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulSelect, SB_QE2000_ELIB_CMAP_PST pMap )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    uint32 ulCmapFwd0, ulCmapFwd1;
    uint32 ulCmapDrp0, ulCmapDrp1;

    DENTER();
    SB_ASSERT( Handle );
    SB_ASSERT( ulSelect < SB_QE2000_ELIB_NUM_CMAPS_K );
    SB_ASSERT( pMap );

    if( 0 == ulSelect )
    {
        ulCmapFwd0 = SAND_HAL_READ( pEp->pHalCtx, KA, EP_BF_FWD_CLASS_MAP0_0 );
        ulCmapFwd1 = SAND_HAL_READ( pEp->pHalCtx, KA, EP_BF_FWD_CLASS_MAP0_1 );

        ulCmapDrp0 = SAND_HAL_READ( pEp->pHalCtx, KA, EP_BF_DROP_CLASS_MAP0_0 );
        ulCmapDrp1 = SAND_HAL_READ( pEp->pHalCtx, KA, EP_BF_DROP_CLASS_MAP0_1 );
    }
    else
    {
        ulCmapFwd0 = SAND_HAL_READ( pEp->pHalCtx, KA, EP_BF_FWD_CLASS_MAP1_0 );
        ulCmapFwd1 = SAND_HAL_READ( pEp->pHalCtx, KA, EP_BF_FWD_CLASS_MAP1_1 );

        ulCmapDrp0 = SAND_HAL_READ( pEp->pHalCtx, KA, EP_BF_DROP_CLASS_MAP1_0 );
        ulCmapDrp1 = SAND_HAL_READ( pEp->pHalCtx, KA, EP_BF_DROP_CLASS_MAP1_1 );
    }

    pMap->ulFwdMap[0]  = SAND_HAL_GET_FIELD(KA, EP_BF_FWD_CLASS_MAP0_0, CNT0, ulCmapFwd0 );
    pMap->ulFwdMap[1]  = SAND_HAL_GET_FIELD(KA, EP_BF_FWD_CLASS_MAP0_0, CNT1, ulCmapFwd0 );
    pMap->ulFwdMap[2]  = SAND_HAL_GET_FIELD(KA, EP_BF_FWD_CLASS_MAP0_0, CNT2, ulCmapFwd0 );
    pMap->ulFwdMap[3]  = SAND_HAL_GET_FIELD(KA, EP_BF_FWD_CLASS_MAP0_0, CNT3, ulCmapFwd0 );
    pMap->ulFwdMap[4]  = SAND_HAL_GET_FIELD(KA, EP_BF_FWD_CLASS_MAP0_0, CNT4, ulCmapFwd0 );
    pMap->ulFwdMap[5]  = SAND_HAL_GET_FIELD(KA, EP_BF_FWD_CLASS_MAP0_0, CNT5, ulCmapFwd0 );
    pMap->ulFwdMap[6]  = SAND_HAL_GET_FIELD(KA, EP_BF_FWD_CLASS_MAP0_0, CNT6, ulCmapFwd0 );
    pMap->ulFwdMap[7]  = SAND_HAL_GET_FIELD(KA, EP_BF_FWD_CLASS_MAP0_0, CNT7, ulCmapFwd0 );

    pMap->ulFwdMap[8]  = SAND_HAL_GET_FIELD(KA, EP_BF_FWD_CLASS_MAP0_1, CNT8,  ulCmapFwd1 );
    pMap->ulFwdMap[9]  = SAND_HAL_GET_FIELD(KA, EP_BF_FWD_CLASS_MAP0_1, CNT9,  ulCmapFwd1 );
    pMap->ulFwdMap[10] = SAND_HAL_GET_FIELD(KA, EP_BF_FWD_CLASS_MAP0_1, CNT10, ulCmapFwd1 );
    pMap->ulFwdMap[11] = SAND_HAL_GET_FIELD(KA, EP_BF_FWD_CLASS_MAP0_1, CNT11, ulCmapFwd1 );
    pMap->ulFwdMap[12] = SAND_HAL_GET_FIELD(KA, EP_BF_FWD_CLASS_MAP0_1, CNT12, ulCmapFwd1 );
    pMap->ulFwdMap[13] = SAND_HAL_GET_FIELD(KA, EP_BF_FWD_CLASS_MAP0_1, CNT13, ulCmapFwd1 );
    pMap->ulFwdMap[14] = SAND_HAL_GET_FIELD(KA, EP_BF_FWD_CLASS_MAP0_1, CNT14, ulCmapFwd1 );
    pMap->ulFwdMap[15] = SAND_HAL_GET_FIELD(KA, EP_BF_FWD_CLASS_MAP0_1, CNT15, ulCmapFwd1 );


    pMap->ulDrpMap[0]  = SAND_HAL_GET_FIELD(KA, EP_BF_DROP_CLASS_MAP0_0, CNT0, ulCmapDrp0 );
    pMap->ulDrpMap[1]  = SAND_HAL_GET_FIELD(KA, EP_BF_DROP_CLASS_MAP0_0, CNT1, ulCmapDrp0 );
    pMap->ulDrpMap[2]  = SAND_HAL_GET_FIELD(KA, EP_BF_DROP_CLASS_MAP0_0, CNT2, ulCmapDrp0 );
    pMap->ulDrpMap[3]  = SAND_HAL_GET_FIELD(KA, EP_BF_DROP_CLASS_MAP0_0, CNT3, ulCmapDrp0 );
    pMap->ulDrpMap[4]  = SAND_HAL_GET_FIELD(KA, EP_BF_DROP_CLASS_MAP0_0, CNT4, ulCmapDrp0 );
    pMap->ulDrpMap[5]  = SAND_HAL_GET_FIELD(KA, EP_BF_DROP_CLASS_MAP0_0, CNT5, ulCmapDrp0 );
    pMap->ulDrpMap[6]  = SAND_HAL_GET_FIELD(KA, EP_BF_DROP_CLASS_MAP0_0, CNT6, ulCmapDrp0 );
    pMap->ulDrpMap[7]  = SAND_HAL_GET_FIELD(KA, EP_BF_DROP_CLASS_MAP0_0, CNT7, ulCmapDrp0 );

    pMap->ulDrpMap[8]  = SAND_HAL_GET_FIELD(KA, EP_BF_DROP_CLASS_MAP0_1, CNT8,  ulCmapDrp1 );
    pMap->ulDrpMap[9]  = SAND_HAL_GET_FIELD(KA, EP_BF_DROP_CLASS_MAP0_1, CNT9,  ulCmapDrp1 );
    pMap->ulDrpMap[10] = SAND_HAL_GET_FIELD(KA, EP_BF_DROP_CLASS_MAP0_1, CNT10, ulCmapDrp1 );
    pMap->ulDrpMap[11] = SAND_HAL_GET_FIELD(KA, EP_BF_DROP_CLASS_MAP0_1, CNT11, ulCmapDrp1 );
    pMap->ulDrpMap[12] = SAND_HAL_GET_FIELD(KA, EP_BF_DROP_CLASS_MAP0_1, CNT12, ulCmapDrp1 );
    pMap->ulDrpMap[13] = SAND_HAL_GET_FIELD(KA, EP_BF_DROP_CLASS_MAP0_1, CNT13, ulCmapDrp1 );
    pMap->ulDrpMap[14] = SAND_HAL_GET_FIELD(KA, EP_BF_DROP_CLASS_MAP0_1, CNT14, ulCmapDrp1 );
    pMap->ulDrpMap[15] = SAND_HAL_GET_FIELD(KA, EP_BF_DROP_CLASS_MAP0_1, CNT15, ulCmapDrp1 );

    DEXIT();
    return( SB_ELIB_OK );
}



sbElibStatus_et sbQe2000ElibPriEnableSet( SB_QE2000_ELIB_HANDLE Handle,
                              uint32 ulPort,
                              bool_t   bEnable )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    uint32 ulBfPriRewrite;

    DENTER();
    SB_ASSERT( Handle );
    SB_ASSERT( ulPort < SB_QE2000_ELIB_NUM_PORTS_K );

    /* coverity[unsigned_compare] */
    if ( (0 <= ulPort) && (32 > ulPort) )
    {
        ulBfPriRewrite = SAND_HAL_READ( pEp->pHalCtx,
                                        KA,
                                        EP_BF_PRI_REWRITE0);

        if( TRUE == bEnable )
        {
            ulBfPriRewrite |= ( 1 << ulPort );
        }
        else
        {
            ulBfPriRewrite &= ~( 1 << ulPort );
        }

        SAND_HAL_WRITE( pEp->pHalCtx,
                        KA,
                        EP_BF_PRI_REWRITE0,
                        ulBfPriRewrite );
    }
    else
    {
        ulBfPriRewrite = SAND_HAL_READ( pEp->pHalCtx,
                                        KA,
                                        EP_BF_PRI_REWRITE1);

        if( TRUE == bEnable )
        {
            ulBfPriRewrite |= ( 1 << ( ulPort - 32 ));
        }
        else
        {
            ulBfPriRewrite &= ~( 1 << ( ulPort - 32 ));
        }

        SAND_HAL_WRITE( pEp->pHalCtx,
                        KA,
                        EP_BF_PRI_REWRITE1,
                        ulBfPriRewrite );
    }

    DEXIT();
    return( SB_ELIB_OK );
}


sbElibStatus_et sbQe2000ElibPriEnableGet( SB_QE2000_ELIB_HANDLE Handle,
                              uint32 ulPort,
                              bool_t   *pbEnable)
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    uint32 ulBfPriRewrite;

    DENTER();
    SB_ASSERT( Handle );
    SB_ASSERT( ulPort < SB_QE2000_ELIB_NUM_PORTS_K );
    SB_ASSERT( pbEnable );

    /* coverity[unsigned_compare] */
    if ( (0 <= ulPort) && (32 > ulPort) )
    {
        ulBfPriRewrite = SAND_HAL_READ( pEp->pHalCtx,
                                        KA,
                                        EP_BF_PRI_REWRITE0);

        if( ( 1 << ulPort ) & ulBfPriRewrite )
        {
            *pbEnable = TRUE;
        }
        else
        {
            *pbEnable = FALSE;
        }
    }
    else
    {
        ulBfPriRewrite = SAND_HAL_READ( pEp->pHalCtx,
                                        KA,
                                        EP_BF_PRI_REWRITE1);

        if( ( 1 << ( ulPort - 32 ) ) & ulBfPriRewrite )
        {
            *pbEnable = TRUE;
        }
        else
        {
            *pbEnable = FALSE;
        }
    }

    DEXIT();
    return( SB_ELIB_OK );
}


sbElibStatus_et sbQe2000ElibPortPriRewriteSet( SB_QE2000_ELIB_HANDLE Handle,
                                               uint32 ulPort,
                                               uint32 pPortPriTable[64])
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    uint32 aulPriIdx[SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_SIZE_IN_BYTES / sizeof(uint32)];
    uint32 aulPriTable[SB_ZF_SB_QE2000_ELIB_PRI_TABLE_SIZE / sizeof(uint32)];
    uint32 ulAddr;
    int nIdx;
    sbZfSbQe2000ElibPriTable_t tPriTable;
    sbElibStatus_et status;
    sbZfSbQe2000ElibPriTableAddr_t tPri;

    DENTER();
    SB_ASSERT( Handle );
    SB_ASSERT( ulPort < SB_QE2000_ELIB_NUM_PORTS_K );

    sal_memset(&tPriTable, 0, sizeof(tPriTable));
    /*
     * Get the base index of the table: ulPort, Cos=0 Dp=0 E=0
     */
    tPri.m_nPort = ulPort;
    tPri.m_nCos  = 0;
    tPri.m_nDp   = 0;
    tPri.m_nEcn  = 0;
    /* coverity[uninit_use_in_call] */
    (void)sbZfSbQe2000ElibPriTableAddr_Pack( &tPri,
                                                     (uint8 *)aulPriIdx,
                                                     SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_SIZE_IN_BYTES );

    aulPriIdx[0] = SAND_HTONL(aulPriIdx[0]);
    ulAddr  = aulPriIdx[0] / 8;

    for(nIdx = 0; nIdx < 64; nIdx++)
    {
        sbZfSbQe2000ElibPriTableSet(&tPriTable, nIdx, pPortPriTable[nIdx]);
    }

    (void)sbZfSbQe2000ElibPriTable_Pack( &tPriTable,
                                                 (uint8 *)aulPriTable,
                                                 0);

    for(nIdx = 0; nIdx < 8; nIdx++)
    {
        /* write the table to pri memory */
        status = sbQe2000ElibPriMemWrite( pEp->pHalCtx, ulAddr + nIdx, aulPriTable[nIdx]);
        if (status)
        {
            DEXIT();
            return( status );
        }
    }

    return(SB_ELIB_OK);

}

sbElibStatus_et sbQe2000ElibPortPriRewriteGet( SB_QE2000_ELIB_HANDLE Handle,
                                               uint32 ulPort,
                                               uint32 pPortPriTable[64])
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    uint32 aulPriIdx[SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_SIZE_IN_BYTES / sizeof(uint32)];
    uint32 aulPriTable[SB_ZF_SB_QE2000_ELIB_PRI_TABLE_SIZE / sizeof(uint32)];
    uint32 ulAddr;
    int nIdx;
    sbZfSbQe2000ElibPriTable_t tPriTable;
    sbElibStatus_et status;
    sbZfSbQe2000ElibPriTableAddr_t tPri;

    DENTER();
    SB_ASSERT( Handle );
    SB_ASSERT( ulPort < SB_QE2000_ELIB_NUM_PORTS_K );

    /*
     * Get the base index of the table: ulPort, Cos=0 Dp=0 E=0
     */
    tPri.m_nPort = ulPort;
    tPri.m_nCos  = 0;
    tPri.m_nDp   = 0;
    tPri.m_nEcn  = 0;
    (void)sbZfSbQe2000ElibPriTableAddr_Pack( &tPri,
                                                     (uint8 *)aulPriIdx,
                                                     SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_SIZE_IN_BYTES );

    aulPriIdx[0] = SAND_HTONL(aulPriIdx[0]);
    ulAddr  = aulPriIdx[0] / 8;

    for(nIdx = 0; nIdx < 8; nIdx++)
    {
        /* write the table to pri memory */
        status = sbQe2000ElibPriMemRead( pEp->pHalCtx, ulAddr + nIdx, &aulPriTable[nIdx]);
        if (status)
        {
            DEXIT();
            return( status );
        }
    }

    sbZfSbQe2000ElibPriTable_Unpack( &tPriTable,
                                             (uint8 *)aulPriTable,
                                             0);

    for(nIdx = 0; nIdx < 64; nIdx++)
    {
        pPortPriTable[nIdx] =
            sbZfSbQe2000ElibPriTableGet( &tPriTable, nIdx);
    }

    return( SB_ELIB_OK );
}


sbElibStatus_et sbQe2000ElibPriRewriteSet( SB_QE2000_ELIB_HANDLE Handle,
                                           uint32 ulPort,
                                           uint32 ulCos,
                                           uint32 ulDp,
                                           uint32 ulEcn,
                                           uint32 ulPri )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    uint32 aulPriIdx[SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_SIZE_IN_BYTES / sizeof(uint32)];
    uint32 ulAddr, ulShift, ulMask, ulData, ulReg;
    sbElibStatus_et status;
    sbZfSbQe2000ElibPriTableAddr_t tPri;

    DENTER();
    SB_ASSERT( Handle );
    SB_ASSERT( ulCos < 8 );
    SB_ASSERT( ulDp < 4 );
    SB_ASSERT( ulEcn < 2 );
    SB_ASSERT( ulPort < SB_QE2000_ELIB_NUM_PORTS_K );

    SB_MEMSET(aulPriIdx, 0, sizeof(aulPriIdx));

    tPri.m_nPort = ulPort;
    tPri.m_nCos = ulCos;
    tPri.m_nDp = ulDp;
    tPri.m_nEcn = ulEcn;

    (void)sbZfSbQe2000ElibPriTableAddr_Pack( &tPri,
                                                     (uint8 *)aulPriIdx,
                                                     SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_SIZE_IN_BYTES );

    /* There are 8 pri entries packed per 24-bit word
     * calculate the physical address of the word. In
     * addition, calculate the required shifting to
     * align the 'pri' entry within the word. This will
     * be used to generate the aligned data and required
     * 3-bit mask for clearing out the field. */
    
    aulPriIdx[0] = SAND_HTONL(aulPriIdx[0]);
    ulAddr  = aulPriIdx[0] / 8;
    ulShift = (aulPriIdx[0] % 8) * 3;
    ulMask  = 0x7 << ulShift;
    ulData  = (ulPri & 0x7) << ulShift;

    /* read the relevant word */
    status = sbQe2000ElibPriMemRead( pEp->pHalCtx, ulAddr, &ulReg);
    if (status)
    {
        DEXIT();
        return( status );
    }

    /* clear out the relevant field
     * and then OR in the data */
    ulReg &= ~ulMask;
    ulReg |= ulData;

    /* write the modified word back out to memory */
    status = sbQe2000ElibPriMemWrite( pEp->pHalCtx, ulAddr, ulReg);

    DEXIT();
    return( status );
}

sbElibStatus_et sbQe2000ElibPriRewriteGet( SB_QE2000_ELIB_HANDLE Handle,
                               uint32 ulPort,
                               uint32 ulCos,
                               uint32 ulDp,
                               uint32 ulEcn,
                               uint32* pulPri )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    uint32 aulPriIdx[SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_SIZE_IN_BYTES / sizeof(uint32)];
    uint32 ulTmp, ulAddr, ulShift;
    sbElibStatus_et status;
    sbZfSbQe2000ElibPriTableAddr_t tPri;

    DENTER();
    SB_ASSERT( Handle );
    SB_ASSERT( ulPort < SB_QE2000_ELIB_NUM_PORTS_K );
    SB_ASSERT( ulCos < 8 );
    SB_ASSERT( ulDp < 4 );
    SB_ASSERT( ulEcn < 2 );
    SB_ASSERT( pulPri );

    SB_MEMSET(aulPriIdx, 0, sizeof(aulPriIdx));

    tPri.m_nPort = ulPort;
    tPri.m_nCos = ulCos;
    tPri.m_nDp = ulDp;
    tPri.m_nEcn = ulEcn;

    (void)sbZfSbQe2000ElibPriTableAddr_Pack( &tPri,
                                                     (uint8 *)aulPriIdx,
                                                     SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_SIZE_IN_BYTES );

    /* There are 8 pri entries packed per 24-bit word
     * calculate the physical address of the word. In
     * addition, calculate the required shifting to
     * align the 'pri' entry */
    
    aulPriIdx[0] = SAND_HTONL(aulPriIdx[0]);
    ulAddr  = aulPriIdx[0] / 8;
    ulShift = (aulPriIdx[0] % 8) * 3;

    status = sbQe2000ElibPriMemRead( pEp->pHalCtx, ulAddr, &ulTmp);
    if (status)
    {
        DEXIT();
        return( status );
    }

    *pulPri = (ulTmp >> ulShift) & 0x7;

    DEXIT();
    return( SB_ELIB_OK );
}

int _sbQe2000ElibMVTSet( sbhandle pHalCtx,
                         SB_QE2000_ELIB_MVT_ENTRY_ST Entry,
                         uint32 ulIndex,
                         SB_QE2000_ELIB_SEM_TRYWAIT_PF pfSemTryWait,
                         SB_QE2000_ELIB_SEM_GIVE_PF pfSemGive,
                         int nSemId,
                         uint32 ulTimeOut,
                         void *pUserData)
{
    sbZfSbQe2000ElibFMVT_t sZfFMVT;
    uint64 ullPortMask;
    int nPort;
    sbElibStatus_et nStatus;
    uint32 aulMvtTuple[SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_SIZE_IN_BYTES / sizeof(uint32)];
    int nGrpSize;
    int nColumn;

    DENTER();

    if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
    {
        nStatus = (*pfSemTryWait)(nSemId, pUserData, ulTimeOut);
        if( 0 != nStatus)
        {
            return( nStatus );
        }
    }

    SB_MEMSET( &sZfFMVT, 0, sizeof(sZfFMVT) );
    /*
     * Get the column info
     */
    nGrpSize = SAND_HAL_READ( pHalCtx, KA, EG_MC_CONFIG0 );
    nGrpSize = SAND_HAL_GET_FIELD( KA, EG_MC_CONFIG0, MCGROUP_SIZE, nGrpSize );
    nColumn = SB_QE2000_ELIB_EB_MVT_COL_GET(ulIndex, nGrpSize);
    SB_ASSERT( nColumn < 3 );
    /*
     * Get the MVT row
     */
    nStatus = sbQe2000ElibMvtEntryReadRaw( pHalCtx, ulIndex, aulMvtTuple );
    if( 0 != nStatus )
    {
        if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
        {
            (*pfSemGive)(nSemId, pUserData);
        }
        return( nStatus );
    }

    sbZfSbQe2000ElibFMVT_Unpack(&sZfFMVT,
                                        (uint8 *)aulMvtTuple,
                                        SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_SIZE_IN_BYTES);

    COMPILER_64_ZERO(ullPortMask);
    for( nPort = 0; nPort < SB_QE2000_ELIB_NUM_PORTS_K; nPort++ )
    {
        if( TRUE == Entry.bPortEnable[nPort] )
        { uint64 nPortMask = COMPILER_64_INIT(0,1);
            COMPILER_64_SHL(nPortMask, nPort);
            COMPILER_64_OR(ullPortMask,nPortMask);
        }
    }


    switch( nColumn )
    {
        case 0:
            sZfFMVT.m_nnPortMap0 = ullPortMask;
            sZfFMVT.m_nMvtda0 = Entry.ulMvtdA;
            sZfFMVT.m_nMvtdb0 = Entry.ulMvtdB;
            sZfFMVT.m_nNext0 = Entry.ulNext;
            if( TRUE == Entry.bSourceKnockout )
            {
                sZfFMVT.m_nKnockout0 = 1;
            }
            else
            {
                sZfFMVT.m_nKnockout0 = 0;
            }


            break;

        case 1:
            sZfFMVT.m_nnPortMap1 = ullPortMask;
            sZfFMVT.m_nMvtda1 = Entry.ulMvtdA;
            sZfFMVT.m_nMvtdb1 = Entry.ulMvtdB;
            sZfFMVT.m_nNext1 = Entry.ulNext;
            if( TRUE == Entry.bSourceKnockout )
            {
                sZfFMVT.m_nKnockout1 = 1;
            }
            else
            {
                sZfFMVT.m_nKnockout1 = 0;
            }
            break;

        case 2:
            sZfFMVT.m_nnPortMap2 = ullPortMask;
            sZfFMVT.m_nMvtda2 = Entry.ulMvtdA;
            sZfFMVT.m_nMvtdb2 = Entry.ulMvtdB;
            sZfFMVT.m_nNext2 = Entry.ulNext;

            if( TRUE == Entry.bSourceKnockout )
            {
                sZfFMVT.m_nKnockout2 = 1;
            }
            else
            {
                sZfFMVT.m_nKnockout2 = 0;
            }
            break;

        default:
            if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
            {
                (*pfSemGive)(nSemId, pUserData);
            }
            return( SB_ELIB_BAD_IDX ); /* should have SB_ASSERTed above */
    }



    (void)sbZfSbQe2000ElibFMVT_Pack( &sZfFMVT,
                                             (uint8 *)aulMvtTuple,
                                             0 );

    nStatus = sbQe2000ElibMvtEntryWriteRaw( pHalCtx, ulIndex, aulMvtTuple );
    if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
    {
        (*pfSemGive)(nSemId, pUserData);
    }

    if( 0 != nStatus )
    {
        return( nStatus );
    }

    return( SB_ELIB_OK );
}


sbElibStatus_et sbQe2000ElibMVTSet( SB_QE2000_ELIB_HANDLE Handle,
                        SB_QE2000_ELIB_MVT_ENTRY_ST Entry,
                        uint32 ulIndex )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    sbElibStatus_et nStatus;

    DENTER();
    SB_ASSERT( Handle );

    nStatus = _sbQe2000ElibMVTSet( pEp->pHalCtx,
                                   Entry,
                                   ulIndex,
                                   pEp->pfUserSemTryWait,
                                   pEp->pfUserSemGive,
                                   pEp->nMVTSemId,
                                   pEp->nMVTTimeOut,
                                   pEp->pMVTUserSemData);

    DEXIT();
    return( nStatus );
}

int _sbQe2000ElibMVTGet( sbhandle pHalCtx,
                         SB_QE2000_ELIB_MVT_ENTRY_PST pEntry,
                         uint32 ulIndex,
                         SB_QE2000_ELIB_SEM_TRYWAIT_PF pfSemTryWait,
                         SB_QE2000_ELIB_SEM_GIVE_PF pfSemGive,
                         int nSemId,
                         uint32 ulTimeOut,
                         void *pUserData)
{
    sbZfSbQe2000ElibFMVT_t sZfFMVT;
    int nPort;
    sbElibStatus_et nStatus;
    uint32 aulMvtTuple[SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_SIZE_IN_BYTES / sizeof(uint32)];
    uint64 ullPortMask;
    int nGrpSize;
    int nColumn;

    DENTER();

    SB_ASSERT( pEntry );

    if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
    {
        nStatus = (*pfSemTryWait)(nSemId, pUserData, ulTimeOut);
        if( 0 != nStatus)
        {
            return( nStatus );
        }
    }

    SB_MEMSET( &sZfFMVT, 0, sizeof(sZfFMVT) );
    SB_MEMSET( pEntry, 0, sizeof(*pEntry) );

    nGrpSize = SAND_HAL_READ( pHalCtx, KA, EG_MC_CONFIG0 );
    nGrpSize = SAND_HAL_GET_FIELD( KA, EG_MC_CONFIG0, MCGROUP_SIZE, nGrpSize );
    nColumn = SB_QE2000_ELIB_EB_MVT_COL_GET(ulIndex, nGrpSize);
    SB_ASSERT( nColumn < 3 );
    /*
     * Get the MVT row
     */
    nStatus = sbQe2000ElibMvtEntryReadRaw( pHalCtx, ulIndex, aulMvtTuple );
    if( 0 != nStatus )
    {
        if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
        {
            (*pfSemGive)(nSemId, pUserData);
        }
        return( nStatus );
    }

    sbZfSbQe2000ElibFMVT_Unpack(&sZfFMVT,
                                        (uint8 *)aulMvtTuple,
                                        SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_SIZE_IN_BYTES);

    switch( nColumn )
    {
        case 0:
            ullPortMask = sZfFMVT.m_nnPortMap0;
            pEntry->ulMvtdA = sZfFMVT.m_nMvtda0;
            pEntry->ulMvtdB = sZfFMVT.m_nMvtdb0;
            pEntry->ulNext  = sZfFMVT.m_nNext0;
            if( TRUE == sZfFMVT.m_nKnockout0 )
            {
                pEntry->bSourceKnockout = 1;
            }
            break;

        case 1:
            ullPortMask = sZfFMVT.m_nnPortMap1;
            pEntry->ulMvtdA = sZfFMVT.m_nMvtda1;
            pEntry->ulMvtdB = sZfFMVT.m_nMvtdb1;
            pEntry->ulNext  = sZfFMVT.m_nNext1;
            if( TRUE == sZfFMVT.m_nKnockout1 )
            {
                pEntry->bSourceKnockout = 1;
            }
            break;

        case 2:
            ullPortMask = sZfFMVT.m_nnPortMap2;
            pEntry->ulMvtdA = sZfFMVT.m_nMvtda2;
            pEntry->ulMvtdB = sZfFMVT.m_nMvtdb2;
            pEntry->ulNext  = sZfFMVT.m_nNext2;
            if( TRUE == sZfFMVT.m_nKnockout2 )
            {
                pEntry->bSourceKnockout = 1;
            }
            break;

        default:
            if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
            {
                (*pfSemGive)(nSemId, pUserData);
            }
            return( SB_ELIB_BAD_IDX ); /* should have SB_ASSERTed above */
    }

    for( nPort = 0; nPort < SB_QE2000_ELIB_NUM_PORTS_K; nPort++ )
    {
      if (COMPILER_64_BITTEST(ullPortMask, nPort))
        {
            pEntry->bPortEnable[nPort] = TRUE;
        }
        else
        {
            pEntry->bPortEnable[nPort] = FALSE;
        }
    }

    if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
    {
        (*pfSemGive)(nSemId, pUserData);
    }

    DEXIT();
    return( SB_ELIB_OK );
}




sbElibStatus_et sbQe2000ElibMVTGet( SB_QE2000_ELIB_HANDLE Handle,
                        SB_QE2000_ELIB_MVT_ENTRY_PST pEntry,
                        uint32 ulIndex )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    sbElibStatus_et nStatus;

    DENTER();
    SB_ASSERT( Handle );
    SB_ASSERT( pEntry );

    nStatus = _sbQe2000ElibMVTGet( pEp->pHalCtx,
                                   pEntry,
                                   ulIndex,
                                   pEp->pfUserSemTryWait,
                                   pEp->pfUserSemGive,
                                   pEp->nMVTSemId,
                                   pEp->nMVTTimeOut,
                                   pEp->pMVTUserSemData);

    DEXIT();
    return( nStatus );
}


sbElibStatus_et sbQe2000ElibMVTPortEnableRMW(sbhandle pHalCtx,
                                 SB_QE2000_ELIB_MVT_ENTRY_ST Entry,
                                 uint32 ulIndex,
                                 SB_QE2000_ELIB_SEM_TRYWAIT_PF pfSemTryWait,
                                 SB_QE2000_ELIB_SEM_GIVE_PF pfSemGive,
                                 int nSemId,
                                 uint32 ulTimeOut,
                                 void *pUserData)
{
    sbZfSbQe2000ElibFMVT_t sZfFMVT;
    int nPort;
    sbElibStatus_et nStatus;
    uint32 aulMvtTuple[SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_SIZE_IN_BYTES / sizeof(uint32)];
    uint64 ullPortMask;
    int nGrpSize;
    int nColumn;

    DENTER();

    if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
    {
        nStatus = (*pfSemTryWait)(nSemId, pUserData, ulTimeOut);
        if( 0 != nStatus)
        {
            return( nStatus );
        }
    }

    SB_MEMSET( &sZfFMVT, 0, sizeof(sZfFMVT) );

    /*
     * Get the column info
     */
    nGrpSize = SAND_HAL_READ( pHalCtx, KA, EG_MC_CONFIG0 );
    nGrpSize = SAND_HAL_GET_FIELD( KA, EG_MC_CONFIG0, MCGROUP_SIZE, nGrpSize );
    nColumn = SB_QE2000_ELIB_EB_MVT_COL_GET(ulIndex, nGrpSize);
    SB_ASSERT( nColumn < 3 );
    /*
     * Get the MVT row
     */
    nStatus = sbQe2000ElibMvtEntryReadRaw( pHalCtx, ulIndex, aulMvtTuple );
    if( 0 != nStatus )
    {
        if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
        {
            (*pfSemGive)(nSemId, pUserData);
        }
        return( nStatus );
    }

    sbZfSbQe2000ElibFMVT_Unpack(&sZfFMVT,
                                        (uint8 *)aulMvtTuple,
                                        SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_SIZE_IN_BYTES);

    COMPILER_64_ZERO(ullPortMask);
    for( nPort = 0; nPort < SB_QE2000_ELIB_NUM_PORTS_K; nPort++ )
    {
        if( TRUE == Entry.bPortEnable[nPort] )
        {uint64 uuTmp = COMPILER_64_INIT(0,1);
            COMPILER_64_SHL(uuTmp, nPort);
            COMPILER_64_OR(ullPortMask, uuTmp);
        }
    }

    switch( nColumn )
    {
        case 0:
            sZfFMVT.m_nnPortMap0 = ullPortMask;
            break;

        case 1:
            sZfFMVT.m_nnPortMap1 = ullPortMask;
            break;

        case 2:
            sZfFMVT.m_nnPortMap2 = ullPortMask;
            break;

        default:
            if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
            {
                (*pfSemGive)(nSemId, pUserData);
            }
            return( SB_ELIB_BAD_IDX ); /* should have SB_ASSERTed above */
    }

    (void)sbZfSbQe2000ElibFMVT_Pack( &sZfFMVT,
                                             (uint8 *)aulMvtTuple,
                                             0 );

    nStatus = sbQe2000ElibMvtEntryWriteRaw( pHalCtx, ulIndex, aulMvtTuple );


    if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
    {
        (*pfSemGive)(nSemId, pUserData);
    }

    DEXIT();
    return( SB_ELIB_OK );
}


sbElibStatus_et sbQe2000ElibMVTMvtdARMW(sbhandle pHalCtx,
                            SB_QE2000_ELIB_MVT_ENTRY_ST Entry,
                            uint32 ulIndex,
                            SB_QE2000_ELIB_SEM_TRYWAIT_PF pfSemTryWait,
                            SB_QE2000_ELIB_SEM_GIVE_PF pfSemGive,
                            int nSemId,
                            uint32 ulTimeOut,
                            void *pUserData)
{
    sbZfSbQe2000ElibFMVT_t sZfFMVT;
    sbElibStatus_et nStatus;
    uint32 aulMvtTuple[SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_SIZE_IN_BYTES / sizeof(uint32)];
    int nGrpSize;
    int nColumn;

    DENTER();

    if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
    {
        nStatus = (*pfSemTryWait)(nSemId, pUserData, ulTimeOut);
        if( SB_ELIB_OK != nStatus)
        {
            return( nStatus );
        }
    }

    SB_MEMSET( &sZfFMVT, 0, sizeof(sZfFMVT) );

    /*
     * Get the column info
     */
    nGrpSize = SAND_HAL_READ( pHalCtx, KA, EG_MC_CONFIG0 );
    nGrpSize = SAND_HAL_GET_FIELD( KA, EG_MC_CONFIG0, MCGROUP_SIZE, nGrpSize );
    nColumn = SB_QE2000_ELIB_EB_MVT_COL_GET(ulIndex, nGrpSize);
    SB_ASSERT( nColumn < 3 );
    /*
     * Get the MVT row
     */
    nStatus = sbQe2000ElibMvtEntryReadRaw( pHalCtx, ulIndex, aulMvtTuple );
    if( SB_ELIB_OK != nStatus )
    {
        if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
        {
            (*pfSemGive)(nSemId, pUserData);
        }
        return( nStatus );
    }

    sbZfSbQe2000ElibFMVT_Unpack(&sZfFMVT,
                                        (uint8 *)aulMvtTuple,
                                        SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_SIZE_IN_BYTES);

    switch( nColumn )
    {
        case 0:
            sZfFMVT.m_nMvtda0 = Entry.ulMvtdA;
            break;

        case 1:
            sZfFMVT.m_nMvtda1 = Entry.ulMvtdA;
            break;

        case 2:
            sZfFMVT.m_nMvtda2 = Entry.ulMvtdA;
            break;

        default:
            if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
            {
                (*pfSemGive)(nSemId, pUserData);
            }
            return( SB_ELIB_BAD_IDX ); /* should have SB_ASSERTed above */
    }

    (void)sbZfSbQe2000ElibFMVT_Pack( &sZfFMVT,
                                             (uint8 *)aulMvtTuple,
                                             0 );

    nStatus = sbQe2000ElibMvtEntryWriteRaw( pHalCtx, ulIndex, aulMvtTuple );


    if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
    {
        (*pfSemGive)(nSemId, pUserData);
    }

    DEXIT();
    return( nStatus );
}

sbElibStatus_et sbQe2000ElibMVTMvtdBRMW(sbhandle pHalCtx,
                            SB_QE2000_ELIB_MVT_ENTRY_ST Entry,
                            uint32 ulIndex,
                            SB_QE2000_ELIB_SEM_TRYWAIT_PF pfSemTryWait,
                            SB_QE2000_ELIB_SEM_GIVE_PF pfSemGive,
                            int nSemId,
                            uint32 ulTimeOut,
                            void *pUserData)
{
    sbZfSbQe2000ElibFMVT_t sZfFMVT;
    sbElibStatus_et nStatus;
    uint32 aulMvtTuple[SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_SIZE_IN_BYTES / sizeof(uint32)];
    int nGrpSize;
    int nColumn;

    DENTER();

    if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
    {
        nStatus = (*pfSemTryWait)(nSemId, pUserData, ulTimeOut);
        if( SB_ELIB_OK != nStatus)
        {
            return( nStatus );
        }
    }

    SB_MEMSET( &sZfFMVT, 0, sizeof(sZfFMVT) );

    /*
     * Get the column info
     */
    nGrpSize = SAND_HAL_READ( pHalCtx, KA, EG_MC_CONFIG0 );
    nGrpSize = SAND_HAL_GET_FIELD( KA, EG_MC_CONFIG0, MCGROUP_SIZE, nGrpSize );
    nColumn = SB_QE2000_ELIB_EB_MVT_COL_GET(ulIndex, nGrpSize);
    SB_ASSERT( nColumn < 3 );
    /*
     * Get the MVT row
     */
    nStatus = sbQe2000ElibMvtEntryReadRaw( pHalCtx, ulIndex, aulMvtTuple );
    if( 0 != nStatus )
    {
        if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
        {
            (*pfSemGive)(nSemId, pUserData);
        }
        return( nStatus );
    }

    sbZfSbQe2000ElibFMVT_Unpack(&sZfFMVT,
                                        (uint8 *)aulMvtTuple,
                                        SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_SIZE_IN_BYTES);

    switch( nColumn )
    {
        case 0:
            sZfFMVT.m_nMvtdb0 = Entry.ulMvtdB;

            break;

        case 1:
            sZfFMVT.m_nMvtdb1 = Entry.ulMvtdB;
            break;

        case 2:
            sZfFMVT.m_nMvtdb2 = Entry.ulMvtdB;
            break;

        default:
            if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
            {
                (*pfSemGive)(nSemId, pUserData);
            }
            return( SB_ELIB_BAD_IDX ); /* should have SB_ASSERTed above */
    }

    (void)sbZfSbQe2000ElibFMVT_Pack( &sZfFMVT,
                                             (uint8 *)aulMvtTuple,
                                             0 );

    nStatus = sbQe2000ElibMvtEntryWriteRaw( pHalCtx, ulIndex, aulMvtTuple );

    if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
    {
        (*pfSemGive)(nSemId, pUserData);
    }

    DEXIT();
    return( SB_ELIB_OK );
}

sbElibStatus_et sbQe2000ElibMVTLinkRMW(sbhandle pHalCtx,
                           SB_QE2000_ELIB_MVT_ENTRY_ST Entry,
                           uint32 ulIndex,
                           SB_QE2000_ELIB_SEM_TRYWAIT_PF pfSemTryWait,
                           SB_QE2000_ELIB_SEM_GIVE_PF pfSemGive,
                           int nSemId,
                           uint32 ulTimeOut,
                           void *pUserData)
{
    sbZfSbQe2000ElibFMVT_t sZfFMVT;
    sbElibStatus_et nStatus;
    uint32 aulMvtTuple[SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_SIZE_IN_BYTES / sizeof(uint32)];
    int nGrpSize;
    int nColumn;

    DENTER();

    if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
    {
        nStatus = (*pfSemTryWait)(nSemId, pUserData, ulTimeOut);
        if( SB_ELIB_OK != nStatus)
        {
            return( nStatus );
        }
    }

    SB_MEMSET( &sZfFMVT, 0, sizeof(sZfFMVT) );
    /*
     * Get the column info
     */
    nGrpSize = SAND_HAL_READ( pHalCtx, KA, EG_MC_CONFIG0 );
    nGrpSize = SAND_HAL_GET_FIELD( KA, EG_MC_CONFIG0, MCGROUP_SIZE, nGrpSize );
    nColumn = SB_QE2000_ELIB_EB_MVT_COL_GET(ulIndex, nGrpSize);
    SB_ASSERT( nColumn < 3 );
    /*
     * Get the MVT row
     */
    nStatus = sbQe2000ElibMvtEntryReadRaw( pHalCtx, ulIndex, aulMvtTuple );
    if( SB_ELIB_OK != nStatus )
    {
        if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
        {
            (*pfSemGive)(nSemId, pUserData);
        }
        return( nStatus );
    }

    sbZfSbQe2000ElibFMVT_Unpack(&sZfFMVT,
                                        (uint8 *)aulMvtTuple,
                                        SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_SIZE_IN_BYTES);

    switch( nColumn )
    {
        case 0:
            sZfFMVT.m_nNext0 = Entry.ulNext;
            break;

        case 1:
            sZfFMVT.m_nNext1 = Entry.ulNext;
            break;

        case 2:
            sZfFMVT.m_nNext2 = Entry.ulNext;
            break;

        default:
            if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
            {
                (*pfSemGive)(nSemId, pUserData);
            }
            return( SB_ELIB_BAD_IDX ); /* should have SB_ASSERTed above */
    }

    (void)sbZfSbQe2000ElibFMVT_Pack( &sZfFMVT,
                                             (uint8 *)aulMvtTuple,
                                             0 );

    nStatus = sbQe2000ElibMvtEntryWriteRaw( pHalCtx, ulIndex, aulMvtTuple );


    if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
    {
        (*pfSemGive)(nSemId, pUserData);
    }

    DEXIT();
    return( SB_ELIB_OK );
}

sbElibStatus_et sbQe2000ElibMVTSrcKnockoutRMW(sbhandle pHalCtx,
                                  SB_QE2000_ELIB_MVT_ENTRY_ST Entry,
                                  uint32 ulIndex,
                                  SB_QE2000_ELIB_SEM_TRYWAIT_PF pfSemTryWait,
                                  SB_QE2000_ELIB_SEM_GIVE_PF pfSemGive,
                                  int nSemId,
                                  uint32 ulTimeOut,
                                  void *pUserData)
{
    sbZfSbQe2000ElibFMVT_t sZfFMVT;
    sbElibStatus_et nStatus;
    uint32 aulMvtTuple[SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_SIZE_IN_BYTES / sizeof(uint32)];
    int nGrpSize;
    int nColumn;

    DENTER();

    if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
    {
        nStatus = (*pfSemTryWait)(nSemId, pUserData, ulTimeOut);
        if( 0 != nStatus)
        {
            return( nStatus );
        }
    }

    SB_MEMSET( &sZfFMVT, 0, sizeof(sZfFMVT) );
    /*
     * Get the column info
     */
    nGrpSize = SAND_HAL_READ( pHalCtx, KA, EG_MC_CONFIG0 );
    nGrpSize = SAND_HAL_GET_FIELD( KA, EG_MC_CONFIG0, MCGROUP_SIZE, nGrpSize );
    nColumn = SB_QE2000_ELIB_EB_MVT_COL_GET(ulIndex, nGrpSize);
    SB_ASSERT( nColumn < 3 );
    /*
     * Get the MVT row
     */
    nStatus = sbQe2000ElibMvtEntryReadRaw( pHalCtx, ulIndex, aulMvtTuple );
    if( 0 != nStatus )
    {
        if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
        {
            (*pfSemGive)(nSemId, pUserData);
        }
        return( nStatus );
    }

    sbZfSbQe2000ElibFMVT_Unpack(&sZfFMVT,
                                        (uint8 *)aulMvtTuple,
                                        SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_SIZE_IN_BYTES);

    switch( nColumn )
    {
        case 0:
            if( TRUE == Entry.bSourceKnockout )
            {
                sZfFMVT.m_nKnockout0 = 1;
            }
            else
            {
                sZfFMVT.m_nKnockout0 = 0;
            }
            break;

        case 1:
            if( TRUE == Entry.bSourceKnockout )
            {
                sZfFMVT.m_nKnockout1 = 1;
            }
            else
            {
                sZfFMVT.m_nKnockout1 = 0;
            }

            break;

        case 2:
            if( TRUE == Entry.bSourceKnockout )
            {
                sZfFMVT.m_nKnockout2 = 1;
            }
            else
            {
                sZfFMVT.m_nKnockout2 = 0;
            }
            break;

        default:
            if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
            {
                (*pfSemGive)(nSemId, pUserData);
            }
            return( SB_ELIB_BAD_IDX ); /* should have SB_ASSERTed above */
    }

    (void)sbZfSbQe2000ElibFMVT_Pack( &sZfFMVT,
                                             (uint8 *)aulMvtTuple,
                                             0 );

    nStatus = sbQe2000ElibMvtEntryWriteRaw( pHalCtx, ulIndex, aulMvtTuple );


    if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
    {
        (*pfSemGive)(nSemId, pUserData);
    }

    DEXIT();
    return( nStatus );
}

sbElibStatus_et sbQe2000ElibMemClear( SB_QE2000_ELIB_HANDLE Handle )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    uint32 ulOffset;
    sbElibStatus_et nStatus;

    DENTER();
    SB_ASSERT( pEp );

    for (ulOffset = 0; ulOffset < SB_QE2000_ELIB_CL_MEM_MAX_OFFSET; ulOffset++)
    {
        nStatus = sbQe2000ElibClMemWrite( pEp->pHalCtx, ulOffset, 0x0, 0x0 );
        if (nStatus)
        {
            DEXIT();
            return( nStatus );
        }
    }

    for (ulOffset = 0; ulOffset < SB_QE2000_ELIB_BF_MEM_MAX_OFFSET; ulOffset++)
    {
        nStatus = sbQe2000ElibBfMemWrite( pEp->pHalCtx, ulOffset, 0x0, 0x0 );
        if (nStatus)
        {
            DEXIT();
            return( nStatus );
        }
    }

    for (ulOffset = 0; ulOffset < SB_QE2000_ELIB_PRI_MEM_MAX_OFFSET; ulOffset++)
    {
        nStatus = sbQe2000ElibPriMemWrite( pEp->pHalCtx, ulOffset, 0x0 );
        if (nStatus)
        {
            DEXIT();
            return( nStatus );
        }
    }

    for (ulOffset = 0; ulOffset < SB_QE2000_ELIB_IP_MEM_MAX_OFFSET; ulOffset++)
    {
        nStatus = sbQe2000ElibIpMemWrite( pEp->pHalCtx, ulOffset, 0x0, 0x0 );
        if (nStatus)
        {
            DEXIT();
            return( nStatus );
        }
    }

    DEXIT();
    return( SB_ELIB_OK );
}


sbElibStatus_et sbQe2000ElibErrorsGet(SB_QE2000_ELIB_HANDLE Handle,
                          SB_QE2000_ELIB_ERROR_PST pErrors)
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    uint32 ulErrors;
    uint32 ulTmp;

    DENTER();
    SB_ASSERT( pEp );
    SB_ASSERT( pErrors );

    SB_MEMSET( pErrors, 0, sizeof(*pErrors ));

    ulErrors = SAND_HAL_READ(pEp->pHalCtx, KA, EP_ERROR);

    if( 0 != SAND_HAL_GET_FIELD(KA, EP_ERROR, BF_PORT_UNMANAGED, ulErrors) )
    {
        pErrors->bBfPortUnmanaged = TRUE;
        ulTmp = SAND_HAL_READ(pEp->pHalCtx, KA, EP_BF_STATUS);
        pErrors->ulUnmanagedVlan = SAND_HAL_GET_FIELD(KA,
                                                      EP_BF_STATUS,
                                                      UNMANAGED_VLAN,
                                                      ulTmp);
        pErrors->ulUnmanagedPort = SAND_HAL_GET_FIELD(KA,
                                                      EP_BF_STATUS,
                                                      UNMANAGED_PORT,
                                                      ulTmp);
    }
    else
    {
        pErrors->bBfPortUnmanaged = FALSE;
    }

    if( 0 != SAND_HAL_GET_FIELD(KA, EP_ERROR, IP_SEG_ERR, ulErrors) )
    {
        pErrors->bIpSegErr = TRUE;
    }
    else
    {
        pErrors->bIpSegErr = FALSE;
    }

    if( 0 != SAND_HAL_GET_FIELD(KA, EP_ERROR, BF_VLAN_DISABLED, ulErrors) )
    {
        pErrors->bBfVlanDisabled = TRUE;
        ulTmp = SAND_HAL_READ(pEp->pHalCtx, KA, EP_BF_STATUS);
        pErrors->ulDisabledVlan = SAND_HAL_GET_FIELD(KA,
                                                     EP_BF_STATUS,
                                                     DISABLED_VLAN,
                                                     ulTmp);
    }
    else
    {
        pErrors->bBfVlanDisabled = FALSE;
    }

    if( 0 != SAND_HAL_GET_FIELD(KA, EP_ERROR, AM_PARITY_ERR, ulErrors) )
    {
        pErrors->bAmParityErr = TRUE;
    }
    else
    {
        pErrors->bAmParityErr = FALSE;
    }

    if( 0 != SAND_HAL_GET_FIELD(KA, EP_ERROR, CL_ISTACK_LIMIT, ulErrors) )
    {
        pErrors->bClIstackLimit = TRUE;
    }
    else
    {
        pErrors->bClIstackLimit = FALSE;
    }

    if( 0 != SAND_HAL_GET_FIELD(KA, EP_ERROR, PF_OVERFLOW, ulErrors) )
    {
        pErrors->bPfOverflow = TRUE;
    }
    else
    {
        pErrors->bPfOverflow = FALSE;
    }

    DEXIT();
    return( SB_ELIB_OK );
}

sbElibStatus_et sbQe2000ElibErrorsClear( SB_QE2000_ELIB_HANDLE Handle,
                             bool_t bBfPortUnmanaged,
                             bool_t bIpSegErr,
                             bool_t bBfVlanDisabled,
                             bool_t bAmParityErr,
                             bool_t bClIstackLimit,
                             bool_t bPfOverflow)
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    uint32 ulErrors;

    DENTER();
    SB_ASSERT( pEp );

    ulErrors = 0;

    if( TRUE == bBfPortUnmanaged )
    {
        ulErrors = SAND_HAL_MOD_FIELD( KA, EP_ERROR, BF_PORT_UNMANAGED, ulErrors, 1);
    }

    if( TRUE == bIpSegErr )
    {
        ulErrors = SAND_HAL_MOD_FIELD(KA, EP_ERROR, IP_SEG_ERR, ulErrors, 1);
    }

    if( TRUE == bBfVlanDisabled )
    {
        ulErrors = SAND_HAL_MOD_FIELD(KA, EP_ERROR, BF_VLAN_DISABLED, ulErrors, 1);
    }

    if( TRUE == bAmParityErr )
    {
        ulErrors = SAND_HAL_MOD_FIELD(KA, EP_ERROR, AM_PARITY_ERR, ulErrors, 1);
    }

    if( TRUE == bClIstackLimit )
    {
        ulErrors = SAND_HAL_MOD_FIELD(KA, EP_ERROR, CL_ISTACK_LIMIT, ulErrors, 1);
    }

    if( TRUE == bPfOverflow )
    {
        ulErrors = SAND_HAL_MOD_FIELD(KA, EP_ERROR, PF_OVERFLOW, ulErrors, 1);
    }

    SAND_HAL_WRITE(pEp->pHalCtx, KA, EP_ERROR, ulErrors);

    DEXIT();
    return( SB_ELIB_OK );
}

sbElibStatus_et sbQe2000ElibClassPktCountGet( SB_QE2000_ELIB_HANDLE Handle,
                                  uint64 pullPktCnt[SB_QE2000_ELIB_NUM_CLASSES_K])
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    int nClass;
    sbElibStatus_et nStatus;

    DENTER();
    SB_ASSERT( pEp );
    SB_ASSERT( pullPktCnt );

    nStatus = sbQe2000ElibSemGet(pEp, SB_QE2000_ELIB_SEM_TO_K);
    if( 0 != nStatus )
    {
        return( nStatus );
    }

    for( nClass = 0; nClass < SB_QE2000_ELIB_NUM_CLASSES_K; nClass++ )
    {
        pullPktCnt[nClass] = pEp->aullClassPktCnt[nClass];
    }

    sbQe2000ElibSemGive(pEp);

    DEXIT();
    return( SB_ELIB_OK );

}


sbElibStatus_et sbQe2000ElibCountsReset( SB_QE2000_ELIB_HANDLE Handle )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    sbElibStatus_et nStatus;
    int nIdx;

    nStatus = sbQe2000ElibSemGet(pEp, SB_QE2000_ELIB_SEM_TO_K);
    if( 0 != nStatus )
    {
        return( nStatus );
    }

    /*
     * Clear the class pkt counters
     */
    for( nIdx = 0; nIdx < SB_QE2000_ELIB_NUM_CLASSES_K; nIdx++ )
    {
        COMPILER_64_ZERO(pEp->aullClassPktCnt[nIdx]);
    }

    for( nIdx = 0; nIdx < SB_QE2000_ELIB_NUM_PORTS_K; nIdx++ )
    {
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_PktClass15);
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_PktClass14);
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_PktClass13);
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_PktClass12);
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_PktClass11);
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_PktClass10);
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_PktClass9 );
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_PktClass8 );
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_PktClass7 );
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_PktClass6 );
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_PktClass5 );
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_PktClass4 );
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_PktClass3 );
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_PktClass2 );
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_PktClass1 );
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_PktClass0 );

        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_ByteClass15);
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_ByteClass14);
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_ByteClass13);
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_ByteClass12);
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_ByteClass11);
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_ByteClass10);
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_ByteClass9 );
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_ByteClass8 );
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_ByteClass7 );
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_ByteClass6 );
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_ByteClass5 );
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_ByteClass4 );
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_ByteClass3 );
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_ByteClass2 );
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_ByteClass1 );
        COMPILER_64_ZERO(pEp->asPCT[nIdx].m_ByteClass0 );
    } /* end for each port */

    sbQe2000ElibVlanCountReset( pEp );

    sbQe2000ElibSemGive( pEp );

    DEXIT();
    return( SB_ELIB_OK );

}


sbElibStatus_et sbQe2000ElibCountsPoll( SB_QE2000_ELIB_HANDLE Handle )
{
    SB_QE2000_ELIB_CONTEXT_PST pEp = (SB_QE2000_ELIB_CONTEXT_PST) Handle;
    sbElibStatus_et status;


    DENTER();
    SB_ASSERT( pEp );

    status = sbQe2000ElibClassPktCountPoll( pEp );
    if( SB_ELIB_OK != status )
    {
        DEXIT();
        return( status );
    }

    status = sbQe2000ElibPCTPoll( pEp );
    if( SB_ELIB_OK != status )
    {
        DEXIT();
        return( status );
    }

    DEXIT();
    return( SB_ELIB_OK );

}

/******************************************************************************
 * Private Functions Below
 ******************************************************************************/
static sbElibStatus_et sbQe2000ElibClassPktCountPoll( SB_QE2000_ELIB_CONTEXT_PST pEp )
{
    int nClass;
    sbElibStatus_et nStatus;

    DENTER();
    SB_ASSERT( pEp );

    nStatus = sbQe2000ElibSemGet(pEp, SB_QE2000_ELIB_SEM_TO_K);
    if( 0 != nStatus )
    {
        return( nStatus );
    }

    for( nClass = 0; nClass < SB_QE2000_ELIB_NUM_CLASSES_K; nClass++ )
    {
        COMPILER_64_ADD_32(pEp->aullClassPktCnt[nClass],SAND_HAL_READ_INDEX(pEp->pHalCtx, KA, EP_CL_PKT0, nClass));
     }

    sbQe2000ElibSemGive(pEp);

    return( SB_ELIB_OK );

}


static uint8 aucPCT[SB_QE2000_ELIB_NUM_CLASSES_K * SB_QE2000_ELIB_NUM_PORTS_K * sizeof(uint64)];

static sbElibStatus_et sbQe2000ElibPCTPoll( SB_QE2000_ELIB_CONTEXT_PST pEp )
{
    sbZfSbQe2000ElibPCT_t sZfPCT;
    sbElibStatus_et status;
    int nPort;
    uint8 *pucPCT;

    DENTER();
    SB_ASSERT( pEp );

    /*
     * Wait for and grab the PCT semaphore
     */
    status = sbQe2000ElibSemGet(pEp, SB_QE2000_ELIB_SEM_TO_K);
    if( SB_ELIB_OK != status )
    {
        return( status );
    }

    /*
     * Run the stats fetch
     */
    if(NULL != pEp->pfUserDmaFunc)
    {
        status = sbQe2000ElibPCTDmaPoll(pEp, aucPCT);
    }
    else
    {
        status = sbQe2000ElibPCTPioPoll(pEp, aucPCT);
    }

    if( SB_ELIB_OK != status )
    {
        /*
         * Release the PCT semaphore
         */
        sbQe2000ElibSemGive(pEp);
        DEXIT();
        return( status );
    }

    for( nPort = 0; nPort < SB_QE2000_ELIB_NUM_PORTS_K; nPort++ )
    {
        pucPCT = &aucPCT[nPort * SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_SIZE];
        sbZfSbQe2000ElibPCT_Unpack(&sZfPCT, pucPCT, 0);

        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_PktClass15, sZfPCT.m_PktClass15);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_PktClass14, sZfPCT.m_PktClass14);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_PktClass13, sZfPCT.m_PktClass13);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_PktClass12, sZfPCT.m_PktClass12);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_PktClass11, sZfPCT.m_PktClass11);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_PktClass10, sZfPCT.m_PktClass10);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_PktClass9, sZfPCT.m_PktClass9);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_PktClass8, sZfPCT.m_PktClass8);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_PktClass7, sZfPCT.m_PktClass7);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_PktClass6, sZfPCT.m_PktClass6);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_PktClass5, sZfPCT.m_PktClass5);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_PktClass4, sZfPCT.m_PktClass4);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_PktClass3, sZfPCT.m_PktClass3);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_PktClass2, sZfPCT.m_PktClass2);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_PktClass1, sZfPCT.m_PktClass1);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_PktClass0, sZfPCT.m_PktClass0);

        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_ByteClass15, sZfPCT.m_ByteClass15);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_ByteClass14, sZfPCT.m_ByteClass14);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_ByteClass13, sZfPCT.m_ByteClass13);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_ByteClass12, sZfPCT.m_ByteClass12);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_ByteClass11, sZfPCT.m_ByteClass11);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_ByteClass10, sZfPCT.m_ByteClass10);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_ByteClass9, sZfPCT.m_ByteClass9);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_ByteClass8, sZfPCT.m_ByteClass8);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_ByteClass7, sZfPCT.m_ByteClass7);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_ByteClass6, sZfPCT.m_ByteClass6);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_ByteClass5, sZfPCT.m_ByteClass5);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_ByteClass4, sZfPCT.m_ByteClass4);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_ByteClass3, sZfPCT.m_ByteClass3);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_ByteClass2, sZfPCT.m_ByteClass2);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_ByteClass1, sZfPCT.m_ByteClass1);
        COMPILER_64_ADD_64(pEp->asPCT[nPort].m_ByteClass0, sZfPCT.m_ByteClass0);

    } /* end for each port */

    sbQe2000ElibSemGive(pEp);

    DEXIT();
    return( SB_ELIB_OK );
}



static sbElibStatus_et sbQe2000ElibPCTDmaPoll( SB_QE2000_ELIB_CONTEXT_PST pEp, uint8 *pucPCT )
{
    sbElibStatus_et status;
    SB_QE2000_ELIB_DMA_FUNC_PARAMS_ST sDmaParams;


    DENTER();
    SB_ASSERT( pEp );
    SB_ASSERT( pEp->pfUserDmaFunc );

    SB_MEMSET(&sDmaParams, 0, sizeof(sDmaParams));

    sDmaParams.StatsMem = SB_QE2000_ELIB_STATS_PCT;
    sDmaParams.ulStartAddr = SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_OFFSET;
    sDmaParams.ulNumLines = 0x320;
    sDmaParams.pData = (void *)pucPCT;
    sDmaParams.ulPreserve = 0; /* Clear on Read */
    sDmaParams.pUserData = pEp->pUserData; /* Pass the user supplied data back to them */
    status = (*pEp->pfUserDmaFunc)((sbhandle)pEp->pHalCtx, &sDmaParams);
    if( 0 != status )
    {
        return( SB_ELIB_DMA_FAIL );
    }

    return( SB_ELIB_OK );
}


static sbElibStatus_et sbQe2000ElibPCTPioPoll( SB_QE2000_ELIB_CONTEXT_PST pEp, uint8 *pucPCT )
{
    sbElibStatus_et status;
    int nMemIdx;
    int nPort;
    int nClass;
    int i;
    uint32 *pulPCT;

    DENTER();
    SB_ASSERT( pEp );
    SB_ASSERT( pucPCT );

    pulPCT = (uint32 *)pucPCT;
    i = 0;
    for( nPort = 0; nPort < SB_QE2000_ELIB_NUM_PORTS_K; nPort++ )
    {
        /*
         * Sweep in the port's class counters.
         */
        for( nClass = 0; nClass < SB_QE2000_ELIB_NUM_CLASSES_K; nClass++ )
        {
            nMemIdx = (nPort * SB_QE2000_ELIB_NUM_CLASSES_K) + nClass;
            nMemIdx += SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_OFFSET;

            status = sbQe2000ElibClMemRead( pEp->pHalCtx,
                                            nMemIdx,
                                            1, /* Clear on Read */
                                            &pulPCT[i],
                                            &pulPCT[i+1] );
            if( SB_ELIB_OK != status)
            {
                DEXIT();
                return( status );
            }

            i += 2;

        }
    }

    DEXIT();
    return( SB_ELIB_OK );
}



#if defined(SB_QE2000_EP_BIST_RUN)
static sbElibStatus_et sbQe2000ElibBistRun( SB_QE2000_ELIB_CONTEXT_PST pEp, uint32 *pulStatus )
{
    uint32 ulBistConfig;
    uint32 ulBistDone;
    int nTimeout;

    DENTER();
    SB_ASSERT( pEp );
    SB_ASSERT( pulStatus );

    ulBistDone = 0;

    ulBistConfig = SAND_HAL_SET_FIELD( KA, EP_BIST_CONFIG0, BIST_SETUP, 2 );

    SAND_HAL_WRITE( pEp->pHalCtx, KA, EP_BIST_CONFIG0, ulBistConfig );

    /*
     * Enable BIST for all memories in the EP
     */
    ulBistConfig =
        SAND_HAL_SET_FIELD( KA, EP_BIST_CONFIG1, BM_BF_MBIST_EN, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_BIST_CONFIG1, PF_CM_MBIST_EN, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_BIST_CONFIG1, PF_D1_MBIST_EN, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_BIST_CONFIG1, PF_D0_MBIST_EN, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_BIST_CONFIG1, AM_CL_MBIST_EN, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_BIST_CONFIG1, MM_BF_MBIST_EN, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_BIST_CONFIG1, MM_IP_MBIST_EN, 1 );



    SAND_HAL_WRITE( pEp->pHalCtx,
                    KA,
                    EP_BIST_CONFIG1,
                    ulBistConfig );

    /*
     * Wait for bist to complete
     */
    nTimeout = 1000;

    while( nTimeout )
    {
        thin_delay( 5000 );
        ulBistDone = SAND_HAL_READ( pEp->pHalCtx,
                                    KA,
                                    EP_BIST_STATUS0 );

        if( ulBistDone == ulBistConfig )
        {
            break;
        }
        else
        {
            nTimeout--;
        }
    }

    if( 0 == nTimeout )
    {
        /*
         * We've timed out.  Return the Done status.
         */
        *pulStatus = ulBistDone;
        DEXIT();
        return(  SB_ELIB_IND_MEM_TIMEOUT );
    }

    /*
     * We didn't time out, but we still need to check the go/no-go
     * status
     */
    ulBistDone = SAND_HAL_READ( pEp->pHalCtx,
                                KA,
                                EP_BIST_STATUS1 );
    /*
     * Turn off bist
     */
    SAND_HAL_WRITE( pEp->pHalCtx, KA, EP_BIST_CONFIG1, 0);


    if( ulBistDone != ulBistConfig )
    {
        /*
         * One or more of the memories did not pass
         */
        *pulStatus = ulBistDone;
        DEXIT();
        return( SB_ELIB_BIST_FAIL );
    }


    *pulStatus = 0;
    DEXIT();
    return( SB_ELIB_OK );

}
#endif

sbElibStatus_et sbQe2000ElibSemGet( SB_QE2000_ELIB_CONTEXT_PST pEp, uint32 nTimeOut )
{
    sbElibStatus_et nStatus;

    SB_ASSERT( pEp );

    if( (NULL != pEp->pfUserSemTryWait) && (-1 != pEp->nStatsSemId))
    {
        nStatus = (*pEp->pfUserSemTryWait)(pEp->nStatsSemId, pEp->pUserSemData, nTimeOut);
        if( 0 != nStatus )
        {
            return( SB_ELIB_SEM_GET_FAIL );
        }
        else
        {
            return( SB_ELIB_OK );
        }
    }

    /*
     * If the functions are NULL we just return OK
     */
    return( SB_ELIB_OK );

}

sbElibStatus_et sbQe2000ElibSemGive( SB_QE2000_ELIB_CONTEXT_PST pEp )
{
    sbElibStatus_et nStatus;

    SB_ASSERT( pEp );

    if( (NULL != pEp->pfUserSemGive) && (-1 != pEp->nStatsSemId))
    {
        nStatus = (*pEp->pfUserSemGive)(pEp->nStatsSemId, pEp->pUserSemData);
        if( 0 != nStatus )
        {
            return( SB_ELIB_SEM_GIVE_FAIL );
        }
        else
        {
            return( SB_ELIB_OK );
        }

    }

    /*
     * If the functions are NULL we just return OK
     */
    return( SB_ELIB_OK );
}
