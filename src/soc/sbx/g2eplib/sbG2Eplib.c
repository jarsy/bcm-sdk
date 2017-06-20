/**
 *
 * ====================================================================
 * ==  sbG2Eplib.c - QE2000 Egress API for Guadalupe2k Ingress UCode
 * ====================================================================
 *
 * WORKING REVISION: $Id: sbG2Eplib.c,v 1.6 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * MODULE NAME:
 *
 *     sbG2Eplib.c
 *
 * ABSTRACT:
 *
 *     QE2000 Egress API for Guadalupe2k Ingress UCode
 *
 * LANGUAGE:
 *
 *     C
 *
 * AUTHORS:
 *
 *     Ning
 *
 * CREATION DATE:
 *
 *     15-April-2007
 *
 */
#include <shared/bsl.h>

#include "soc/drv.h"
#include "soc/sbx/sbx_drv.h"
#include "sbG2Eplib.h"
#include "sbG2EplibI.h"
#include "sbG2EplibCt.h"
#include "sbG2EplibTable.h"
#include "sbQe2000ElibZf.h"
#include "sbQe2000ElibMem.h"

/**-------------------------------------------------------------------------
 * Define on a per-class basis what the payload can be assumed to be.
 * This will match the type of instructions that are implemented per-class
 * to operate and re-assemble the frame. This has dependency on the elib
 * instruction set (file: elib/ucode/sbG2EplibUcode.eg)
 *--------------------------------------------------------------------------*/
static sbG2EplibClassHwTypes_t s_eIpHwTypes[16] =
{
    EN_CLS_HW_TYPE_VLAN, /* class 00 - Traditional Bridging */
    EN_CLS_HW_TYPE_LI,   /* class 01 - Logical Interface */
    EN_CLS_HW_TYPE_VLAN, /* class 02 - Unused */
    EN_CLS_HW_TYPE_VLAN, /* class 03 - Unused */
    EN_CLS_HW_TYPE_VLAN, /* class 05 - Unused */
    EN_CLS_HW_TYPE_VLAN, /* class 05 - Unused */
    EN_CLS_HW_TYPE_VLAN, /* class 06 - Unused */
    EN_CLS_HW_TYPE_VLAN, /* class 07 - Unused */
    EN_CLS_HW_TYPE_VLAN, /* class 08 - Unused */
    EN_CLS_HW_TYPE_VLAN, /* class 09 - Unused */
    EN_CLS_HW_TYPE_VLAN, /* class 10 - Unused */
    EN_CLS_HW_TYPE_VLAN, /* class 11 - Unused */
    EN_CLS_HW_TYPE_VLAN, /* class 12 - Unused */
    EN_CLS_HW_TYPE_VLAN, /* class 13 - Unused */
    EN_CLS_HW_TYPE_VLAN, /* class 14 - Unused */
    EN_CLS_HW_TYPE_VLAN, /* class 15 - Unused */
};

extern bool_t sbG2EplibIsClassValid(uint8 nClass);


/**-------------------------------------------------------------------------
 * sbG2EplibDefParamsGet()
 *
 *  Guadalupe2k Specific Egress Processor Parameter Initialization
 *
 *  pParams - initialization parameters structure to be initialized
 *  returns - error code, zero on success
 *--------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibDefParamsGet(sbG2EplibInitParams_pst pParams)
{
    int nSeg;

    SB_ASSERT( pParams );
    SB_MEMSET(pParams, 0, sizeof(*pParams));
    pParams->nPorts = SB_QE2000_ELIB_NUM_PORTS_K;
    COMPILER_64_SET(pParams->ullERHForward, 0x20000, 0); /* PCI always gets ERH */
    pParams->nOnePcwEna = 0;

    /*
     * Pre load the default ip segment memory tables
     */
    for(nSeg = 0; nSeg < SEG_SEGMENTS; nSeg++) {
        pParams->tIpSegment[nSeg].start     = sbG2EplibGetSegBaseDefault(nSeg);
        pParams->tIpSegment[nSeg].end       = sbG2EplibGetSegEndDefault(nSeg);
        pParams->tIpSegment[nSeg].width     = sbG2EplibGetSegWidthDefault(nSeg);
        pParams->tIpSegment[nSeg].entrysize = sbG2EplibGetSegEntrySizeDefault(nSeg);
    }

    return SB_ELIB_OK;
}


/**-------------------------------------------------------------------------
 * sbG2EplibIpSegmentTableSet()
 *
 *  Set Ip Segment Table entry information in initialization parameters.
 *  Must be called AFTER sbG2EplibDefParamsGet as DefParamsGet initializes
 *  the segment information to default values.
 *
 *  pParams - initialization parameters structure to be initialized
 *  nSeg    - segment to operation on.
 *  pZf     - pointer to a zframe that contains new segment memory information.
 *  returns - error code, zero on success
 *--------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibIpSegmentTableSet(sbG2EplibInitParams_pst pParams, sbG2EplibIpSegs_et nSeg,
                          sbZfG2EplibIpSegment_t *pZf)
{

    pParams->tIpSegment[nSeg].start     =  pZf->start;
    pParams->tIpSegment[nSeg].end       =  pZf->end;
    pParams->tIpSegment[nSeg].width     =  pZf->width;
    pParams->tIpSegment[nSeg].entrysize =  pZf->entrysize;

    return SB_ELIB_OK;
}

/**-------------------------------------------------------------------------
 * sbG2EplibInit()
 *
 *  Guadalupe2k Specific Egress Processor Initialization. sbG2EplibDefParamsGet
 *  MUST be called before this function as it defines the ip segment memory
 *  table.
 *
 *  pCtxt   - handle to elib context structure
 *  pParams - initialization parameters structure
 *  returns - error code, zero on success
 *--------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibInit(sbG2EplibCtxt_pst pCtxt, sbG2EplibInitParams_pst pParams)
{
    sbElibStatus_et nStatus;
    SB_QE2000_ELIB_CMAP_ST sCmap;
    SB_QE2000_ELIB_INIT_PARAMS_ST sEpInitParams;
    int nClass;
    int nSeg;
    int nPort;

    SB_ASSERT(pParams);

    /* Get Default Params for ELib */
    sbQe2000ElibDefParamsGet(&sEpInitParams);
    sEpInitParams.pUserData = pParams->pUserData;
    sEpInitParams.pfUserDmaFunc = pParams->pUserDmaFunc;
    sEpInitParams.nPorts = pParams->nPorts;
    sEpInitParams.pfUserSemCreate = pParams->pUserSemCreateFunc;
    sEpInitParams.pfUserSemGive = pParams->pUserSemGiveFunc;
    sEpInitParams.pfUserSemTryWait = pParams->pUserSemWaitGetFunc;
    sEpInitParams.pfUserSemDestroy = pParams->pUserSemDestroyFunc;
    sEpInitParams.pUserSemData = pParams->pUserSemData;
    sEpInitParams.pMVTUserSemData = pParams->pMVTUserSemData;
    sEpInitParams.nMVTSemId = pParams->nMVTSemId;
    sEpInitParams.nMVTTimeOut = pParams->nMVTTimeOut;
    sEpInitParams.ullERHForward = pParams->ullERHForward;
    sEpInitParams.MmConfig = SB_QE2000_ELIB_MM_CONFIG_BF0;
    sEpInitParams.bBypassBF = TRUE;
    sEpInitParams.bBypassIP = FALSE;
    sEpInitParams.bBypassCL = FALSE;
    /*    sEpInitParams.ullSmacMsbEnable = pParams->ullSmacMsbEnable;  */
    /*    sEpInitParams.bOnePcwEna = pParams->nOnePcwEna;              */
    pCtxt->pHalCtx = pParams->pHalCtx;
    pCtxt->bPCTCounts = TRUE;

    for(nSeg = 0; nSeg < SEG_SEGMENTS; nSeg++) {
        pCtxt->tIpSegment[nSeg].start     = pParams->tIpSegment[nSeg].start;
        pCtxt->tIpSegment[nSeg].end       = pParams->tIpSegment[nSeg].end;
        pCtxt->tIpSegment[nSeg].width     = pParams->tIpSegment[nSeg].width;
        pCtxt->tIpSegment[nSeg].entrysize =  pParams->tIpSegment[nSeg].entrysize;
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    if (SOC_WARM_BOOT((int)pCtxt->pHalCtx)){
      return SOC_E_NONE;
    }
#endif

    /* Initialize EP library */
    nStatus = sbQe2000ElibInit(&pCtxt->EpHandle, pParams->pHalCtx, sEpInitParams);
    if (nStatus)
        return nStatus;

    /* Setup Class Instruction Table */
    nStatus = sbG2EplibCITInit(pCtxt);
    if (SB_ELIB_OK != nStatus)
        return nStatus;

    /* Setup Class Resolution Table */
    nStatus = sbG2EplibCRTInit(pCtxt->EpHandle);
    if (SB_ELIB_OK != nStatus)
        return nStatus;

    /* Syncronize the CRT table to Chip Memory */
    nStatus = sbG2EplibCRTCommit(pCtxt, TRUE);
    if (SB_ELIB_OK != nStatus)
        return nStatus;

    /* Setup Memory Segments */
    nStatus = sbG2EplibSegmentInit(pCtxt);
    if (SB_ELIB_OK != nStatus)
        return nStatus;

    /* Setup Class Hw Types */
    nStatus = sbG2EplibClassTypesSet(pParams->pHalCtx, s_eIpHwTypes);
    if (SB_ELIB_OK != nStatus)
        return nStatus;

    /* Setup Default Configurations */
    for (nClass = 0; nClass < SB_QE2000_ELIB_NUM_CLASSES_K; nClass++) {
        /*
         * Map all class counters:
         *  All forwarded go to counter 0
         *  All dropped go to counter 1
         */
        sCmap.ulFwdMap[nClass] = 0;
        sCmap.ulDrpMap[nClass] = 1;
    }

    nStatus = sbQe2000ElibCmapSet(pCtxt->EpHandle, 0, sCmap);
    if (SB_ELIB_OK != nStatus)
        return nStatus;

    /* Setup QE2k-Ep Ports */
    for (nPort = 0; nPort < SB_QE2000_ELIB_NUM_PORTS_K; nPort++) {

        nStatus = sbG2EplibPortConfigSet(pCtxt, nPort);
        if (SB_ELIB_OK != nStatus)
        return nStatus;
    }

    /* Configure the Expected header types on a per-class basis */
    SAND_HAL_WRITE(pParams->pHalCtx, KA, EP_IP_HW, 0);


    /*
     * Setup the counter map for map 1
     *
     *  Class 0 & 1 map to counter 0 (Bridging)
     *  Class 2, 3, 4, 8, 9 & 11 map to counter 1 (IP)
     *  Class 12 & 13 map to counter 2 (MPLS)
     *
     *  All drops map to counter 3, as well as the disabled
     *  Classes.
     */
    for (nClass = 0; nClass < SB_QE2000_ELIB_NUM_CLASSES_K; nClass++) {
        sCmap.ulDrpMap[nClass] = 3;
    }

    /* Bridging Counters */
    sCmap.ulFwdMap[0] = 0;
    sCmap.ulFwdMap[1] = 0;
    /* IP Counters */
    sCmap.ulFwdMap[2]  = 1;
    sCmap.ulFwdMap[3]  = 1;
    sCmap.ulFwdMap[4]  = 1;
    sCmap.ulFwdMap[8]  = 1;
    sCmap.ulFwdMap[9]  = 1;
    sCmap.ulFwdMap[11] = 1;
    /* MPLS Counters */
    sCmap.ulFwdMap[12] = 2;
    sCmap.ulFwdMap[13] = 2;
    /* Unused Classes */
    sCmap.ulFwdMap[5] = 2;
    sCmap.ulFwdMap[6] = 2;
    sCmap.ulFwdMap[7] = 2;
    sCmap.ulFwdMap[10] = 2;
    sCmap.ulFwdMap[14] = 2;
    sCmap.ulFwdMap[15] = 2;

    nStatus = sbQe2000ElibCmapSet(pCtxt->EpHandle, 1, sCmap);
    if (SB_ELIB_OK != nStatus)
        return nStatus;

    /* Return Success */
    return SB_ELIB_OK;
}

/**-------------------------------------------------------------------------
 * sbG2EplibUnInit()
 *
 *  Guadalupe2k Specific Egress Processor Uninitialization
 *
 *  pCtxt   - handle to elib context structure
 *  returns - error code, zero on success
 *--------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibUnInit(sbG2EplibCtxt_pst pCtxt)
{
    sbElibStatus_et nStatus;
    nStatus = sbQe2000ElibUnInit(pCtxt->EpHandle);
    return nStatus;
}

/**-------------------------------------------------------------------------
 * sbG2EplibHQosRemapSet()
 *
 *  Define an entry in the H-Qos Table that is used to compose
 *  a packet. This table is located in
 *  the IP memory, and contains Vlan Information that will be used later by
 *  the Bridging Function (BF)
 *
 *  pCtxt   - Pointer to the Egress context structure
 *  ulIdx   - Index into the H-Qos Table in the IP memory
 *  pData   - buffer containing the HQos table entry.
 *  returns - error code, zero on success
 *--------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibHQosRemapSet(sbG2EplibCtxt_pst pCtxt, uint32 ulIdx,
		      uint32* pData)
{
    sbElibStatus_et nStatus;

    SB_ASSERT(pCtxt);

    nStatus = sbG2EplibIpTableWrite(pCtxt, SEG_HQOS_REMAP, ulIdx, pData);

    return nStatus;
}
/**-------------------------------------------------------------------------
 * sbG2EplibHQosRemapGet()
 *
 *  Retrieve an entry in the HQos Table that is used to compose
 *  a packet. This table is located in
 *  the IP memory, and contains Vlan Information that will be used later by
 *  the Bridging Function (BF)
 *
 *  pCtxt   - Pointer to the Egress context structure
 *  ulIdx   - Index into the H-Qos Table in the IP memory
 *  *pData  - buffer to return the H-Qos table entry
 *  returns - error code, zero on success
 *--------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibHQosRemapGet(sbG2EplibCtxt_pst pCtxt, uint32 ulIdx,
		      uint32* pData)
{
    sbElibStatus_et nStatus;

    SB_ASSERT(pCtxt);

    nStatus  = sbG2EplibIpTableRead(pCtxt, SEG_HQOS_REMAP, ulIdx, pData);
    if (SB_ELIB_OK != nStatus)
        return nStatus;

    return SB_ELIB_OK;
}

/**-------------------------------------------------------------------------
 * sbG2EplibVlanRemapSet()
 *
 *  Define an entry in the VlanRemap Table that is used to compose
 *  a packet. This table is located in
 *  the IP memory, and contains Vlan Information that will be used later by
 *  the Bridging Function (BF)
 *
 *  pCtxt   - Pointer to the Egress context structure
 *  ulIdx   - Index into the VlanRemap Table in the IP memory
 *  pData   - buffer containing the vlan remap table entry.
 *  returns - error code, zero on success
 *--------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibVlanRemapSet(sbG2EplibCtxt_pst pCtxt, uint32 ulIdx,
		      uint32* pData)
{
    sbElibStatus_et nStatus;

    SB_ASSERT(pCtxt);

    nStatus = sbG2EplibIpTableWrite(pCtxt, SEG_VLAN_REMAP, ulIdx, pData);

    return nStatus;
}
/**-------------------------------------------------------------------------
 * sbG2EplibVlanRemapGet()
 *
 *  Retrieve an entry in the VlanRemap Table that is used to compose
 *  a packet. This table is located in
 *  the IP memory, and contains Vlan Information that will be used later by
 *  the Bridging Function (BF)
 *
 *  pCtxt   - Pointer to the Egress context structure
 *  ulIdx   - Index into the VlanRemap Table in the IP memory
 *  *pData  - buffer to return the VlanRemap table entry
 *  returns - error code, zero on success
 *--------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibVlanRemapGet(sbG2EplibCtxt_pst pCtxt, uint32 ulIdx,
		      uint32* pData)
{
    sbElibStatus_et nStatus;

    SB_ASSERT(pCtxt);

    nStatus  = sbG2EplibIpTableRead(pCtxt, SEG_VLAN_REMAP, ulIdx, pData);
    if (SB_ELIB_OK != nStatus)
        return nStatus;

    return SB_ELIB_OK;
}

/**-------------------------------------------------------------------------
 * sbG2EplibPortEncapSet()
 *
 *  Define an entry in the TciDMAC Table that is used to compose
 *  a packet. This table is located in
 *  the IP memory, and contains Vlan Information that will be used later by
 *  the Bridging Function (BF)
 *
 *  pCtxt   - Pointer to the Egress context structure
 *  ulIdx   - Index into the TciDMAC Table in the IP memory
 *  pData   - buffer containing the PortEncap table entry.
 *  returns - error code, zero on success
 *--------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibPortEncapSet(sbG2EplibCtxt_pst pCtxt, uint32 ulIdx,
		      uint32* pData)
{
    sbElibStatus_et nStatus;

    SB_ASSERT(pCtxt);

    nStatus = sbG2EplibIpTableWrite(pCtxt, SEG_PORT_REMAP, ulIdx, pData);

    return nStatus;
}
/**-------------------------------------------------------------------------
 * sbG2EplibPortEncapGet()
 *
 *  Retrieve an entry in the PortEncap Table that is used to compose
 *  a packet. This table is located in
 *  the IP memory, and contains port encap Information that will be used later by
 *  the Bridging Function (BF)
 *
 *  pCtxt   - Pointer to the Egress context structure
 *  ulIdx   - Index into the PortEncap Table in the IP memory
 *  *pData  - buffer to return the PortEncap table entry
 *  returns - error code, zero on success
 *--------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibPortEncapGet(sbG2EplibCtxt_pst pCtxt, uint32 ulIdx,
		      uint32* pData)
{
    sbElibStatus_et nStatus;

    SB_ASSERT(pCtxt);

    nStatus  = sbG2EplibIpTableRead(pCtxt, SEG_PORT_REMAP, ulIdx, pData);
    if (SB_ELIB_OK != nStatus)
        return nStatus;

    return SB_ELIB_OK;
}

/**-------------------------------------------------------------------------
 * sbG2EplibPortConfigSet()
 *
 *  Define a port configuration. This is used to setup a port on the egress
 *
 *  pCtxt  - Pointer to the Egress context structure
 *  ulPort - Port number associated with this configuration
 *  status - Error code, zero on success
 *--------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibPortConfigSet(sbG2EplibCtxt_pst pCtxt, uint32 ulPort)
{
    SB_QE2000_ELIB_PORT_CFG_ST portCfg;
    int nClass, nStatus;

    SB_ASSERT(pCtxt);
    SB_ASSERT(ulPort < SB_QE2000_ELIB_NUM_PORTS_K);

    SB_MEMSET(&portCfg, 0, sizeof(portCfg));

    /* Enable all classes defined in the CRT */
    for (nClass = 0; nClass < SB_QE2000_ELIB_NUM_CLASSES_K; nClass++) {
        portCfg.bClassEna[nClass] = sbG2EplibIsClassValid(nClass);
    }

    portCfg.bAppend        = 0;    /* 1 = Append, 0 = Prepend */
    portCfg.bInstValid     = 0;    /* 1 = Valid, 0 = Not valid */
    portCfg.ulInst         = 0;    /* EP Instruction per port (disabled) */
    portCfg.bPriRewriteEna = 0;    /* PriRewriteEnable */
    portCfg.bBfEna         = 0;    /* Enable Bridging Function */
    portCfg.bUntaggedEna   = 0;    /* Enable tranmission of untagged frames (actually, strip tag func) */
    portCfg.TagSel         = 0;    /* Vlan tag used in encapsulation */

    nStatus = sbQe2000ElibPortCfgSet(pCtxt->EpHandle, ulPort, portCfg);

    if (SB_ELIB_OK != nStatus)
        return nStatus;

    return SB_ELIB_OK;
}

/**-------------------------------------------------------------------------
 * sbG2EplibPortConfigGet()
 *
 *  Retrieve a port's configuration.
 *
 *  pCtxt  - Pointer to the Egress context structure.
 *  ulPort - Port number associated with this configuration.
 *  status - Error code, zero on success
 *--------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibPortConfigGet(sbG2EplibCtxt_pst pCtxt, uint32 ulPort)
{
    SB_QE2000_ELIB_PORT_CFG_ST portCfg;
    int nStatus;

    SB_ASSERT(pCtxt);
    SB_ASSERT(ulPort < SB_QE2000_ELIB_NUM_PORTS_K);

    nStatus = sbQe2000ElibPortCfgGet(pCtxt->EpHandle, ulPort, &portCfg);
    if (SB_ELIB_OK != nStatus)
        return nStatus;

    return SB_ELIB_OK;
}


/**-------------------------------------------------------------------------
 * sbG2EplibMVTSet()
 *
 *  Set an Multicast Vector Table Entry
 *
 *  pCtxt     - handle to the elib context structure
 *  ulMcGroup - The MVT entry index.
 *  pZf       - Pointer to an MVT entry zframe.
 *  returns   - Error code, zero on success
 *--------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibMVTSet(sbG2EplibCtxt_pst pCtxt, uint32 ulMcGroup,
               sbZfG2EplibMvtEntry_t *pZf)
{
    SB_QE2000_ELIB_MVT_ENTRY_ST sMvtEntry;
    int nStatus;
    int nPort;

    SB_ASSERT(pCtxt);
    SB_ASSERT(pZf);

    /*
     * Do a RMW, not touching the port enables
     */
    nStatus = sbQe2000ElibMVTGet(pCtxt->EpHandle, &sMvtEntry, ulMcGroup);
    if(SB_ELIB_OK != nStatus) {
        return nStatus;
    }

    for( nPort = 0; nPort < SB_QE2000_ELIB_NUM_PORTS_K; nPort++ )
    {
      if(COMPILER_64_BITTEST(pZf->ullPortMask, nPort)) {
	sMvtEntry.bPortEnable[nPort] = TRUE;
      }
      else {
	sMvtEntry.bPortEnable[nPort] = FALSE;
      }
    }

    sMvtEntry.ulMvtdA = pZf->ulMvtdA;
    sMvtEntry.ulMvtdB = pZf->ulMvtdB;

    sMvtEntry.bSourceKnockout = pZf->bSourceKnockout;

    /* BUG 21863 */
    if(TRUE == pZf->bEnableChaining) {
        sMvtEntry.ulNext = pZf->ulNextMcGroup;
    } else {
        sMvtEntry.ulNext = 0xFFFF;
    }

    nStatus = sbQe2000ElibMVTSet(pCtxt->EpHandle, sMvtEntry, ulMcGroup);
    if (SB_ELIB_OK != nStatus)
	return nStatus;

    return SB_ELIB_OK;
}

/**-------------------------------------------------------------------------
 * sbG2EplibMVTGet()
 *
 *  Get an Multicast Vector Table Entry
 *
 *  pCtxt     - handle to the elib context structure.
 *  ulMcGroup - The MVT entry index.
 *  pZf       - The MVT entry stored at the given index. Upon success, the MVT
 *              entry will be placed at this location.
 *  returns   - Error code, zero on success
 *--------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibMVTGet(sbG2EplibCtxt_pst pCtxt, uint32 ulMcGroup,
               sbZfG2EplibMvtEntry_t *pZf)
{
    SB_QE2000_ELIB_MVT_ENTRY_ST sMvtEntry;
    int nStatus;
    int nPort;

    SB_ASSERT(pCtxt);
    SB_ASSERT(pZf);

    SB_MEMSET(pZf, 0, sizeof(*pZf));

    nStatus = sbQe2000ElibMVTGet(pCtxt->EpHandle, &sMvtEntry, ulMcGroup);
    if (SB_ELIB_OK != nStatus)
	return nStatus;

    pZf->ulMvtdA = 0x3FFF & sMvtEntry.ulMvtdA;
    pZf->ulMvtdB = 0x000F & sMvtEntry.ulMvtdB;

    pZf->bSourceKnockout = sMvtEntry.bSourceKnockout;
    /* BUG 21863 */
    if(0xFFFF != sMvtEntry.ulNext) {
        pZf->bEnableChaining = TRUE;
        pZf->ulNextMcGroup = sMvtEntry.ulNext;
    } else {
        pZf->bEnableChaining = FALSE;
        pZf->ulNextMcGroup = 0xFFFF;
    }

    COMPILER_64_ZERO(pZf->ullPortMask);

    for( nPort = 0; nPort < SB_QE2000_ELIB_NUM_PORTS_K; nPort++ )
    {
        if(TRUE == sMvtEntry.bPortEnable[nPort]) {
            if (nPort < 32) {
                COMPILER_64_SET(pZf->ullPortMask, 
                                COMPILER_64_HI(pZf->ullPortMask), 
                                COMPILER_64_LO(pZf->ullPortMask) |
                                (1 << nPort));
            } else {
                COMPILER_64_SET(pZf->ullPortMask, 
                                COMPILER_64_HI(pZf->ullPortMask) |
                                (1 << (nPort-32)), 
                                COMPILER_64_LO(pZf->ullPortMask));
            }
        }
    }

    return SB_ELIB_OK;
}


/**-------------------------------------------------------------------------
 * sbG2EplibCRTCommit()
 *
 *  Push previously set CRT entries to the device.
 *  The Class Resolution Table (CRT) is indexed by a 10-bit value that is
 *  user-defined. This function synchronizes the local copy of the CRT with
 *  the device.
 *
 *  pCtxt   - handle to the elib context structure
 *  bAll    - Syncronized all CRT entries, not just the lines marked as dirty.
 *  returns - Error code, zero on success
 *--------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibCRTCommit(sbG2EplibCtxt_pst pCtxt, bool_t bAll)
{
    int nStatus;

    SB_ASSERT(pCtxt);

    nStatus = sbQe2000ElibCRTPush(pCtxt->EpHandle, bAll);
    if (SB_ELIB_OK != nStatus)
	return nStatus;

    return SB_ELIB_OK;
}

/**-------------------------------------------------------------------------
 * sbG2EplibClassPktCountGet()
 *
 *  Retrieve per class packet count
 *
 *  pCtxt     - handle to the elib context structure.
 *  pulPktCnt - upon success the packet count for each class
 *  returns   - Error code, zero on success
 *--------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibClassPktCountGet(sbG2EplibCtxt_pst pCtxt,
			 uint64 pullPktCnt[SB_QE2000_ELIB_NUM_CLASSES_K])
{
    int nStatus;

    SB_ASSERT(pCtxt);

    nStatus = sbQe2000ElibClassPktCountGet(pCtxt->EpHandle, pullPktCnt);
    if (SB_ELIB_OK != nStatus)
	return nStatus;

    return SB_ELIB_OK;
}

/**-------------------------------------------------------------------------
 * sbG2EplibVlanFlush()
 *
 *  Remove all VLAN entries, reinitialize associated memory
 *
 *  pCtxt     - handle to the elib context structure.
 *  returns   - Error code, zero on success
 *--------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibVlanFlush(sbG2EplibCtxt_pst pCtxt)
{
    int nStatus;

    SB_ASSERT(pCtxt);

    nStatus = sbQe2000ElibVlanFlush(pCtxt->EpHandle);
    if (SB_ELIB_OK != nStatus)
	return nStatus;

    return SB_ELIB_OK;
}


/**-------------------------------------------------------------------------
 * sbG2EplibCountsReset()
 *
 *  Clear all counters.
 *
 *  pCtxt     - handle to the elib context structure.
 *  returns   - Error code, zero on success
 *--------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibCountersReset(sbG2EplibCtxt_pst pCtxt)
{
    int nStatus;

    SB_ASSERT(pCtxt);

    nStatus = sbQe2000ElibCountsReset(pCtxt->EpHandle);
    if (SB_ELIB_OK != nStatus)
	return nStatus;

    return SB_ELIB_OK;
}


sbG2EplibCtxt_st *
gu2_unit2ep(int unit)
{
    soc_sbx_control_t *sbx;

    sbx = SOC_SBX_CONTROL(unit);
    if (sbx == NULL) {
        return NULL;
    }

    if (!SOC_IS_SBX_QE(unit)) {
        LOG_CLI((BSL_META_U(unit,
                            "\n unit2ep allowed for QE only")));
        return NULL;
    }

    return (sbG2EplibCtxt_st *) sbx->drv;
}
