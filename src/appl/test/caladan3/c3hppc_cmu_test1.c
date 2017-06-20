/*
 * $Id: c3hppc_cmu_test1.c,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */ 

#include <shared/bsl.h>

#include <soc/defs.h>


#ifdef BCM_CALADAN3_SUPPORT

#include "../c3hppc_test.h"
 
#define C3HPPC_CMU_TEST1__SEGMENT_NUM  30
#define C3HPPC_CMU_TEST1__STREAM_NUM   5

static c3hppc_64b_ocm_entry_template_t *g_pFlowTable;
static int g_nFlowTableSize;

static uint32 g_uSegmentType, g_uPacketSizeShift;


int
c3hppc_cmu_test1__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  int rc, nCopInstance;
  int nSegment, nStartingPhysicalBlock1, nStartingPhysicalBlock0;
  c3hppc_cmu_segment_info_t *pCmuSegmentInfo;
  

  if ( SAL_BOOT_QUICKTURN ) {
    g_uSegmentType = ( sal_rand() % 2 ) ? C3HPPC_CMU_SEGMENT_TYPE__TURBO_32b :
                                          C3HPPC_CMU_SEGMENT_TYPE__TURBO_64b;
    g_uPacketSizeShift = ( g_uSegmentType == C3HPPC_CMU_SEGMENT_TYPE__TURBO_64b ) ? 4 : 0;
  } else {
    g_uSegmentType = C3HPPC_CMU_SEGMENT_TYPE__TURBO_64b;
    g_uPacketSizeShift = 0; 
  }

  c3hppc_populate_with_defaults( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );
  if ( pc3hppcTestInfo->nSwitchCount == 2 ) {
    strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "cmu_test1a_switch2.oasm");
    COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,4);
  } else if ( pc3hppcTestInfo->nSwitchCount == 1 ) {
    strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "cmu_test1a_switch1.oasm");
    COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,5);
  } else {
    strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "cmu_test1a.oasm");
  }

  nStartingPhysicalBlock0 = 32;
  nStartingPhysicalBlock1 = 32;
  for ( nSegment = 0; nSegment < C3HPPC_CMU_TEST1__SEGMENT_NUM; ++nSegment ) {
    pCmuSegmentInfo = &(pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo[nSegment]);
    pCmuSegmentInfo->bValid = 1;
    pCmuSegmentInfo->uSegment = (uint32) nSegment;
    pCmuSegmentInfo->uSegmentOcmBase = (nSegment >> 1) * C3HPPC_OCM_MAX_PHY_BLK_SIZE_IN_ROWS;
    pCmuSegmentInfo->uSegmentType = g_uSegmentType; 
    pCmuSegmentInfo->uSegmentLimit = ((pc3hppcTestInfo->bBankSwap ? 2 : 1) * C3HPPC_TEST_SIMPLEX_FLOW_TABLE_SIZE) - 1;
    pCmuSegmentInfo->uSegmentPort = nSegment & 1;
    pCmuSegmentInfo->nStartingPhysicalBlock = ( nSegment ) ? nStartingPhysicalBlock1++ :
                                                             nStartingPhysicalBlock0++;
  }

  rc = c3hppc_bringup( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );
  if ( rc ) return 1;

  for ( nCopInstance = 0; nCopInstance < C3HPPC_COP_INSTANCE_NUM; ++nCopInstance ) {
    c3hppc_cop_segments_enable( pc3hppcTestInfo->nUnit, nCopInstance,
                                pc3hppcTestInfo->BringUpControl.aCopSegmentInfo[nCopInstance] );
  }

  c3hppc_cmu_segments_enable( pc3hppcTestInfo->nUnit,
                              pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo );
  c3hppc_cmu_segments_ejection_enable( pc3hppcTestInfo->nUnit,
                                       pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo );

  return 0;
}

int
c3hppc_cmu_test1__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{

  int nOcmPort;
  int nIndex;
  c3hppc_cmu_segment_info_t *pCmuSegmentInfo;
  char sUcodeFileName[64];
  int nTargetBankSelect;
  int nEpochSize;
  uint32 uReg, uContexts;

  g_nFlowTableSize = 1024;
  g_pFlowTable = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                                                   g_nFlowTableSize * sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                   "flow_table");
  sal_memset( g_pFlowTable, 0, (g_nFlowTableSize * sizeof(c3hppc_64b_ocm_entry_template_t)) );
  /****************************************************************************************************************************
   * A FlowTable entry contains the parameter to select the stream of CMGR operations.
   *               
   *  3:0  --> contains the stream index which is a value between 0 and 4
   *  7:4  --> contains the packet size shift amount
   *****************************************************************************************************************************/
  for ( nIndex = 0; nIndex < g_nFlowTableSize; ++nIndex ) {
    g_pFlowTable[nIndex].uData[0] = ( sal_rand() % C3HPPC_CMU_TEST1__STREAM_NUM ) | ( g_uPacketSizeShift << 4 );
  }
  nOcmPort = c3hppc_ocm_map_cop2ocm_port(0);
  c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, 0, 0, (g_nFlowTableSize-1), 1, g_pFlowTable[0].uData );

  READ_LRA_CONFIG1r( pc3hppcTestInfo->nUnit, &uReg );
  uContexts = soc_reg_field_get( pc3hppcTestInfo->nUnit, LRA_CONFIG1r, uReg, CONTEXTSf );
  COMPILER_64_SET(pc3hppcTestInfo->uuEpochCount, COMPILER_64_HI(pc3hppcTestInfo->uuIterations), COMPILER_64_LO(pc3hppcTestInfo->uuIterations));
  COMPILER_64_UMUL_32(pc3hppcTestInfo->uuEpochCount, C3HPPC_CMU_TEST1__STREAM_NUM);
  COMPILER_64_UMUL_32(pc3hppcTestInfo->uuEpochCount, uContexts);
  { uint64 uuTmp;
    COMPILER_64_SET(uuTmp, COMPILER_64_HI(pc3hppcTestInfo->uuEpochCount), COMPILER_64_LO(pc3hppcTestInfo->uuEpochCount));
    COMPILER_64_ADD_64(uuTmp, pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition);
    c3hppc_lrp_start_control( pc3hppcTestInfo->nUnit, uuTmp);
    cli_out("\nIssued LRP \"start\" command!\n");
  }

  /*
   * Setup manual ejection activity and LRP instruction bank swapping
  */
  /* coverity[secure_coding] */
  sal_strcpy( sUcodeFileName, pc3hppcTestInfo->BringUpControl.sUcodeFileName );
  nTargetBankSelect = 1;
  nEpochSize = 0;
  while ( !c3hppc_is_test_done( pc3hppcTestInfo->nUnit ) ) {
    pCmuSegmentInfo = pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo + ( sal_rand() % C3HPPC_CMU_TEST1__SEGMENT_NUM );
    c3hppc_cmu_segment_flush( pc3hppcTestInfo->nUnit, pCmuSegmentInfo );

    if ( pc3hppcTestInfo->bBankSwap ) {
      sUcodeFileName[9] = ( nTargetBankSelect ) ? 'b' : 'a';
      if ( !c3hppc_lrp_load_ucode(pc3hppcTestInfo->nUnit, sUcodeFileName, nTargetBankSelect,
                                  pc3hppcTestInfo->BringUpControl.bLrpLoaderEnable,
                                  pc3hppcTestInfo->BringUpControl.bLrpMaximizeActiveContexts, &nEpochSize) ) {
        c3hppc_lrp_bank_swap( pc3hppcTestInfo->nUnit );
        nTargetBankSelect ^= 1;
        c3hppc_lrp_bank_corrupt( pc3hppcTestInfo->nUnit, nTargetBankSelect );
      }
    }

    assert( !c3hppc_cmu_display_error_state(pc3hppcTestInfo->nUnit) );
  }

  cli_out("\nDetected LRP \"offline\" event\n");


  return 0;
}

int
c3hppc_cmu_test1__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
#if !defined(COMPILER_HAS_DOUBLE) || !defined(COMPILER_HAS_LONGLONG)

  cli_out("\nTHIS TEST DOES NOT SUPPORT THIS PLATFORM/OS!!!!! \n");
  return SOC_E_UNAVAIL;

#else
  int nErrorCnt;
  uint64 *pPktCounterA, *pBytCounterA;
  uint64 *pPktCounterB, *pBytCounterB;
  uint64 uuPktCounterB, uuBytCounterB;
  int nSegment;
  uint32 uCounterId;
  uint64 uuExpectPktCount, uuExpectBytCount;
  c3hppc_cmu_segment_info_t *pCmuSegmentInfo;
  uint32 uReg, uContexts;



  /****************************************************************************************************************************
   * When the LRP context number is 8, it takes 8 epochs to cycle through 1K FlowIds.  Therefore with 5 streams it takes
   * 40 (5*8) epochs to rotate through the cross product of streams and FlowIds.
   * When the LRP context number is 7, it takes 7 epochs to cycle through 896(7*128) FlowIds.  Therefore with 5 streams it takes
   * 35 (5*7) epochs to rotate through the cross product of streams and FlowIds.
   *****************************************************************************************************************************/
  READ_LRA_CONFIG1r( pc3hppcTestInfo->nUnit, &uReg );
  uContexts = soc_reg_field_get( pc3hppcTestInfo->nUnit, LRA_CONFIG1r, uReg, CONTEXTSf );

  COMPILER_64_ZERO(uuPktCounterB);
  COMPILER_64_ZERO(uuBytCounterB);
  pBytCounterB = &uuBytCounterB;
  pPktCounterB = &uuPktCounterB;
  nErrorCnt = 0;
  for ( nSegment = 0; nSegment < C3HPPC_CMU_SEGMENT_NUM; ++nSegment ) {
    pCmuSegmentInfo = pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo + nSegment;
    if ( pCmuSegmentInfo->bValid ) {
      for ( uCounterId = 0; uCounterId < C3HPPC_TEST_SIMPLEX_FLOW_TABLE_SIZE; ++uCounterId ) {
        uint64 uuTmp;
        if ( uContexts == 7 && (uCounterId >> 7) == 7 ) continue;
        pBytCounterA = pCmuSegmentInfo->pSegmentPciBase + (2 * uCounterId);
        pPktCounterA = pBytCounterA + 1;
        if ( pc3hppcTestInfo->bBankSwap ) {
          pBytCounterB = pCmuSegmentInfo->pSegmentPciBase + (2 * (C3HPPC_TEST_SIMPLEX_FLOW_TABLE_SIZE + uCounterId));
          pPktCounterB = pBytCounterB + 1;
        }
        COMPILER_64_SET(uuExpectPktCount, COMPILER_64_HI(pc3hppcTestInfo->uuIterations), COMPILER_64_LO(pc3hppcTestInfo->uuIterations));
        COMPILER_64_SET(uuExpectBytCount, COMPILER_64_HI(uuExpectPktCount), COMPILER_64_LO(uuExpectPktCount));
        COMPILER_64_UMUL_32(uuExpectBytCount, (uCounterId + nSegment));
        COMPILER_64_SHL(uuExpectBytCount, g_uPacketSizeShift);
        COMPILER_64_SET(uuTmp, COMPILER_64_HI(*pPktCounterA), COMPILER_64_LO(*pPktCounterA));
        COMPILER_64_ADD_64(uuTmp, *pPktCounterB);
        if ( COMPILER_64_NE(uuTmp, uuExpectPktCount) ||
             (pc3hppcTestInfo->bBankSwap && (COMPILER_64_IS_ZERO(*pPktCounterA) || COMPILER_64_IS_ZERO(*pPktCounterB))) ) {
          if ( nErrorCnt < 128 ) {
            cli_out("<c3hppc_cmu_test1__done> -- Packet count MISCOMPARE for Segment[%d] CountID[%d]  Actual: 0x%llx   Expect: 0x%llx \n",
                    nSegment, uCounterId, uuTmp, uuExpectPktCount );
            if ( pc3hppcTestInfo->bBankSwap ) {
              cli_out("<c3hppc_cmu_test1__done> -- pPktCounterA: 0x%llx   pPktCounterB: 0x%llx \n",
                      *pPktCounterA, *pPktCounterB );
            }
          }
          ++nErrorCnt;
        }
        COMPILER_64_SET(uuTmp, COMPILER_64_HI(*pBytCounterA), COMPILER_64_LO(*pBytCounterA));
        COMPILER_64_ADD_64(uuTmp, *pBytCounterB);
        if ( COMPILER_64_NE(uuTmp, uuExpectBytCount) ) {
          if ( nErrorCnt < 128 ) {
            cli_out("<c3hppc_cmu_test1__done> -- Packet byte MISCOMPARE for Segment[%d] CountID[%d]  Actual: 0x%llx   Expect: 0x%llx \n",
                    nSegment, uCounterId, uuTmp, uuExpectBytCount );
            if ( pc3hppcTestInfo->bBankSwap ) {
              cli_out("<c3hppc_cmu_test1__done> -- pBytCounterA: 0x%llx   pBytCounterB: 0x%llx \n",
                      *pBytCounterA, *pBytCounterB );
            }
          }
          ++nErrorCnt;
        }
      }
    }
  }

  if ( nErrorCnt ) {
    pc3hppcTestInfo->nTestStatus = TEST_FAIL;
    cli_out("\n<c3hppc_cmu_test1__done> -- Total error count --> %d\n", nErrorCnt );
  }

  READ_CM_EF_MAX_DEPTHr( pc3hppcTestInfo->nUnit, &uReg );
  cli_out("\n<c3hppc_cmu_test1__done> -- CM_EF_MAX_DEPTH --> %d\n", uReg );

  soc_cm_sfree(pc3hppcTestInfo->nUnit, g_pFlowTable);

  return 0;
#endif
}

#endif /* #ifdef BCM_CALADAN3_SUPPORT */
