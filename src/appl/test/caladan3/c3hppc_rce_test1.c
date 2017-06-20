/* $Id: c3hppc_rce_test1.c,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/defs.h>


#ifdef BCM_CALADAN3_SUPPORT

#include "../c3hppc_test.h"
 
#define C3HPPC_RCE_TEST1__STREAM_NUM                (1)
#define C3HPPC_RCE_TEST1__MAX_PROGRAM_NUM           (4)

static c3hppc_64b_ocm_entry_template_t *g_pFlowTable;
static int g_nFlowTableSize;

static uint32 g_auResultsRuleCountConfigLo[C3HPPC_RCE_RESULTS_COUNTER_NUM];
static uint32 g_auResultsRuleCountConfigHi[C3HPPC_RCE_RESULTS_COUNTER_NUM];
static uint32 g_uFilterSetToMove;
static uint64 g_auuResultsRuleCountersSnapShot[C3HPPC_RCE_RESULTS_COUNTER_NUM];
static uint32 g_auLpmProgramFilterSetNumber[C3HPPC_RCE_TEST1__MAX_PROGRAM_NUM];
static uint32 g_auLpmProgramFilterSetLength[C3HPPC_RCE_TEST1__MAX_PROGRAM_NUM];
static uint32 g_auLpmProgramKeyLength[C3HPPC_RCE_TEST1__MAX_PROGRAM_NUM];
static uint32 g_auLpmProgramKeyStartIndex[C3HPPC_RCE_TEST1__MAX_PROGRAM_NUM];
static int g_nNumberOfPrograms;


int
c3hppc_rce_test1__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  int rc, nCopInstance, nProgram;
  
  sal_memset( g_auuResultsRuleCountersSnapShot, 0x00, sizeof(g_auuResultsRuleCountersSnapShot) );

  c3hppc_populate_with_defaults( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );
  pc3hppcTestInfo->BringUpControl.uRceBringUp = 1;

  if ( pc3hppcTestInfo->nProgramSelect == 0 ) {
    g_nNumberOfPrograms = ( sal_rand() % 2 ) ? 1 : 4;
  } else {
    g_nNumberOfPrograms = pc3hppcTestInfo->nProgramSelect;
  }

  if ( g_nNumberOfPrograms == 1 ) {

    strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "rce_test1.oasm");
    COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,5);
    g_auLpmProgramFilterSetNumber[0] = 9;
    g_auLpmProgramFilterSetLength[0] = 456;
    g_auLpmProgramKeyLength[0] = 432;
    g_auLpmProgramKeyStartIndex[0] = 0;

  } else if ( g_nNumberOfPrograms == 3 ) {

    strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "rce_test1_3lookups.oasm");
    COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,3);
    for ( nProgram = 0; nProgram < g_nNumberOfPrograms; ++nProgram ) {
      g_auLpmProgramFilterSetNumber[nProgram] = ( nProgram == 0 ) ? 3 : 2;
      g_auLpmProgramFilterSetLength[nProgram] = 512;
      g_auLpmProgramKeyLength[nProgram] = 432;
      g_auLpmProgramKeyStartIndex[nProgram] = 0;
    }

  } else {
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
c3hppc_rce_test1__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{

  int nOcmPort;
  int nIndex, nKeyIndex;
  uint32 uReg, uContexts, uFlowID, uOcmOffset;
  uint64 auuKeyData[7], uuTmp;
  c3hppc_64b_ocm_entry_template_t OcmEntry;
  uint32 uFilterSetIndex, uSBlkIndex, uColumnIndex;
  uint32 auRceResults[4];
  uint32 uFilterSetIndex0, uFilterSetIndex1;
  int nCounterId;
  int nProgramNumber;
  uint32 uProgramBaseAddress, uFilterSetNewLocation;
  uint32 uRegisterValue;
  uint32 uErrorInjectKeyNum;
  int nDoSkips, nDoNoMatches;
  int nLpmProgram;

  nProgramNumber = 0;
  uFilterSetNewLocation = 0;

  g_nFlowTableSize = 1024;
  g_pFlowTable = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                                                   g_nFlowTableSize * sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                   "flow_table");
  sal_memset( g_pFlowTable, 0, (g_nFlowTableSize * sizeof(c3hppc_64b_ocm_entry_template_t)) );
  /****************************************************************************************************************************
   * A FlowTable entry contains the parameter to select the stream of CMGR operations.
   *               
   *  0:0  --> Control bit to enable random skipping of key launches
   *  1:1  --> Control bit to enable key modifications to cause random "no match" results
   *****************************************************************************************************************************/
  /* Error inject with skipping causes the TSR(E) to get set but not reset. */
  if ( pc3hppcTestInfo->nErrorInject == 1 || (sal_rand() % 4) ) {
    nDoSkips = 0;
  } else {
    nDoSkips = 1;
    cli_out("\nRandom skipping of lookups enabled!\n");
  }
  if ( sal_rand() % 4 ) {
    nDoNoMatches = 0;
  } else {
    nDoNoMatches = 2;
    cli_out("\nRandom lookups resulting in \"No Match\" enabled!\n");
  }

  for ( nIndex = 0; nIndex < g_nFlowTableSize; ++nIndex ) {
    g_pFlowTable[nIndex].uData[0] = nDoSkips | nDoNoMatches;
  }
  nOcmPort = c3hppc_ocm_map_cop2ocm_port(0);
  c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, 0, 0, (g_nFlowTableSize-1), 1, g_pFlowTable[0].uData );


  /****************************************************************************************************************************
   * Build RCE program and filter pattern image.
   *****************************************************************************************************************************/
  for ( nLpmProgram = 0; nLpmProgram < g_nNumberOfPrograms; ++nLpmProgram ) {

    nProgramNumber = 10 + nLpmProgram;
    uProgramBaseAddress = (C3HPPC_RCE_INSTRUCTION_NUM / C3HPPC_RCE_TEST1__MAX_PROGRAM_NUM) * nLpmProgram;
    uFilterSetNewLocation = (C3HPPC_RCE_INSTRUCTION_NUM / 2);

    c3hppc_rce_create_program_for_lpm_exact_match( pc3hppcTestInfo->nUnit, nProgramNumber, uProgramBaseAddress,
                                                   g_auLpmProgramFilterSetNumber[nLpmProgram],
                                                   g_auLpmProgramFilterSetLength[nLpmProgram],
                                                   g_auLpmProgramKeyLength[nLpmProgram],
                                                   g_auLpmProgramKeyStartIndex[nLpmProgram],
                                                   C3HPPC_RCE_RESULT_REGS_USE_ALL );

    c3hppc_lrp_setup_rce_program( pc3hppcTestInfo->nUnit, nProgramNumber, (uint32) nProgramNumber );

    COMPILER_64_SET(auuKeyData[0],0x11111111,0x00000000);
    COMPILER_64_SET(auuKeyData[1],0x22222222,0x00000000);
    COMPILER_64_SET(auuKeyData[2],0x33333333,0x00000000);
    COMPILER_64_SET(auuKeyData[3],0x44444444,0x00000000);
    COMPILER_64_SET(auuKeyData[4],0x55555555,0x00000000);
    COMPILER_64_SET(auuKeyData[5],0x66666666,0x00000000);
    COMPILER_64_SET(auuKeyData[6],0x00007777,0x00000000);

    nCounterId = 0;
    uFilterSetIndex0 = sal_rand() % g_auLpmProgramFilterSetNumber[nLpmProgram];
    uFilterSetIndex1 = sal_rand() % g_auLpmProgramFilterSetNumber[nLpmProgram];
    if ( uFilterSetIndex0 == uFilterSetIndex1 ) {
      uFilterSetIndex0 = 0;
      uFilterSetIndex1 = g_auLpmProgramFilterSetNumber[nLpmProgram] - 1;
    }
    cli_out("\nCreating %dx%d LPM program with assigned number [%d] with hits in either FilterSetIndex0[%d] or FilterSetIndex1[%d]  ...\n",
            g_auLpmProgramFilterSetNumber[nLpmProgram], g_auLpmProgramFilterSetLength[nLpmProgram], nProgramNumber, 
            uFilterSetIndex0, uFilterSetIndex1 );

    g_uFilterSetToMove = uFilterSetIndex0;

    for ( uFlowID = 0; uFlowID < g_nFlowTableSize; ++uFlowID ) {

      uOcmOffset = ((uint32)nLpmProgram << 12) | uFlowID;

      for ( nKeyIndex = 0; nKeyIndex < COUNTOF(auuKeyData); ++nKeyIndex ) {
        uint64 uuFlId, uuMask = COMPILER_64_INIT(0xffffffff,0x00000000);
        nOcmPort = c3hppc_ocm_map_lrp2ocm_port(nKeyIndex+1);
        COMPILER_64_AND(auuKeyData[nKeyIndex], uuMask);
        COMPILER_64_SET(uuFlId, 0, uFlowID);
        COMPILER_64_SET(uuTmp, 0, nProgramNumber);
        COMPILER_64_SHL(uuTmp, 28);
        COMPILER_64_OR(uuTmp, uuFlId);
        COMPILER_64_OR(auuKeyData[nKeyIndex], uuTmp);

        OcmEntry.uData[1] = COMPILER_64_HI(auuKeyData[nKeyIndex]);
        OcmEntry.uData[0] = COMPILER_64_LO(auuKeyData[nKeyIndex]);
        c3hppc_ocm_mem_read_write( pc3hppcTestInfo->nUnit, nOcmPort, C3HPPC_DATUM_SIZE_QUADWORD,
                                   uOcmOffset, 1, OcmEntry.uData );
      }

      if ( uFlowID < C3HPPC_RCE_TOTAL_COLUMN_NUM ) {
        uFilterSetIndex = uFilterSetIndex0;
        uSBlkIndex = uFlowID >> 5;
        uColumnIndex = uFlowID & (C3HPPC_RCE_NUM_COLUMNS_PER_SBLOCK - 1);
      } else {
        uFilterSetIndex = uFilterSetIndex1;
        uSBlkIndex = (uFlowID - C3HPPC_RCE_TOTAL_COLUMN_NUM) >> 5;
        uColumnIndex = (uFlowID - C3HPPC_RCE_TOTAL_COLUMN_NUM) & (C3HPPC_RCE_NUM_COLUMNS_PER_SBLOCK - 1);
      }

      c3hppc_rce_add_filter_for_lpm_exact_match( g_auLpmProgramFilterSetLength[nLpmProgram], g_auLpmProgramKeyLength[nLpmProgram],
                                                 uFilterSetIndex, uSBlkIndex, uColumnIndex, uProgramBaseAddress, auuKeyData );

      sal_memset( auRceResults, 0xff, sizeof(auRceResults) );
      auRceResults[(uFilterSetIndex & 3)] = (uFilterSetIndex << 12) | (uSBlkIndex << 5) | uColumnIndex;
      if ( (uFlowID % 128) == 0 ) {
        g_auResultsRuleCountConfigLo[nCounterId] = auRceResults[(uFilterSetIndex & 3)];
        g_auResultsRuleCountConfigHi[nCounterId] = auRceResults[(uFilterSetIndex & 3)];
        c3hppc_rce_conifg_results_counters( pc3hppcTestInfo->nUnit, nCounterId,
                                            g_auResultsRuleCountConfigLo[nCounterId],
                                            g_auResultsRuleCountConfigHi[nCounterId] );
  
        g_auResultsRuleCountConfigLo[nCounterId+8] = 0x80000 | auRceResults[(uFilterSetIndex & 3)];
        g_auResultsRuleCountConfigHi[nCounterId+8] = 0x80000 | auRceResults[(uFilterSetIndex & 3)];
        c3hppc_rce_conifg_results_counters( pc3hppcTestInfo->nUnit, (nCounterId+8),
                                            g_auResultsRuleCountConfigLo[nCounterId+8],
                                            g_auResultsRuleCountConfigHi[nCounterId+8] );
        ++nCounterId;
      }

      for ( nIndex = nKeyIndex; nIndex < (nKeyIndex + 2); ++nIndex ) {
        OcmEntry.uData[0] = ( nIndex == nKeyIndex ) ? auRceResults[0] : auRceResults[2];
        OcmEntry.uData[1] = ( nIndex == nKeyIndex ) ? auRceResults[1] : auRceResults[3];
        nOcmPort = c3hppc_ocm_map_lrp2ocm_port(nIndex+1);
        c3hppc_ocm_mem_read_write( pc3hppcTestInfo->nUnit, nOcmPort, C3HPPC_DATUM_SIZE_QUADWORD,
                                   uOcmOffset, 1, OcmEntry.uData );
      }

    } /* for ( uFlowID = 0; uFlowID < g_nFlowTableSize; ++uFlowID ) { */

    for ( nKeyIndex = 0; nKeyIndex < COUNTOF(auuKeyData); ++nKeyIndex ) {
      COMPILER_64_SET(auuKeyData[nKeyIndex], 0xaaaaaaaa,0xaaaaaaaa);
    }
    for ( uFilterSetIndex = 0; uFilterSetIndex < g_auLpmProgramFilterSetNumber[nLpmProgram]; ++uFilterSetIndex ) {
      if ( uFilterSetIndex == uFilterSetIndex0 || uFilterSetIndex == uFilterSetIndex1 ) continue;
      for ( uFlowID = 0; uFlowID < C3HPPC_RCE_TOTAL_COLUMN_NUM; ++uFlowID ) {
        uSBlkIndex = uFlowID >> 5;
        uColumnIndex = uFlowID & (C3HPPC_RCE_NUM_COLUMNS_PER_SBLOCK - 1);
        c3hppc_rce_add_filter_for_lpm_exact_match( g_auLpmProgramFilterSetLength[nLpmProgram], g_auLpmProgramKeyLength[nLpmProgram],
                                                   uFilterSetIndex, uSBlkIndex, uColumnIndex, uProgramBaseAddress, auuKeyData );
      }
    }

  }  /* for ( nLpmProgram = 0; nLpmProgram < g_nNumberOfPrograms; ++nLpmProgram ) { */

  cli_out("\nLoading the RCE image ...\n");
  c3hppc_rce_dma_image( pc3hppcTestInfo->nUnit );




  READ_LRA_CONFIG1r( pc3hppcTestInfo->nUnit, &uReg );
  uContexts = soc_reg_field_get( pc3hppcTestInfo->nUnit, LRA_CONFIG1r, uReg, CONTEXTSf );
  COMPILER_64_SET(pc3hppcTestInfo->uuEpochCount, COMPILER_64_HI(pc3hppcTestInfo->uuIterations), COMPILER_64_LO(pc3hppcTestInfo->uuIterations));
  COMPILER_64_UMUL_32(pc3hppcTestInfo->uuEpochCount, C3HPPC_RCE_TEST1__STREAM_NUM * uContexts);
  COMPILER_64_SET(uuTmp, COMPILER_64_HI(pc3hppcTestInfo->uuEpochCount), COMPILER_64_LO(pc3hppcTestInfo->uuEpochCount));
  COMPILER_64_ADD_64(uuTmp, pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition);
  c3hppc_lrp_start_control( pc3hppcTestInfo->nUnit, uuTmp );
  cli_out("\nIssued LRP \"start\" command!\n");

  if ( COMPILER_64_LO(pc3hppcTestInfo->uuIterations) >= 10000 && g_nNumberOfPrograms == 1 ) {
    sal_sleep(sal_rand() % 10);
    c3hppc_rce_move_filter_set( pc3hppcTestInfo->nUnit, nProgramNumber, g_auLpmProgramFilterSetLength[0], 
                                g_uFilterSetToMove, uFilterSetNewLocation );
    sal_sleep(1);
    c3hppc_rce_read_results_counters( pc3hppcTestInfo->nUnit );
    for ( nCounterId = 0; nCounterId < C3HPPC_RCE_RESULTS_COUNTER_NUM; ++nCounterId ) {
      g_auuResultsRuleCountersSnapShot[nCounterId] = c3hppc_rce_get_results_counter(nCounterId);
    }
  }

  while ( !c3hppc_is_test_done( pc3hppcTestInfo->nUnit ) ) {
    sal_sleep(1);

    if ( pc3hppcTestInfo->nErrorInject ) {
      uErrorInjectKeyNum = sal_rand() % 128;
      c3hppc_lrp_write_shared_register( pc3hppcTestInfo->nUnit, 0, (0x80000000 | uErrorInjectKeyNum) );

      READ_RC_GLOBAL_DEBUGr( pc3hppcTestInfo->nUnit, &uRegisterValue );
      soc_reg_field_set( pc3hppcTestInfo->nUnit, RC_GLOBAL_DEBUGr, &uRegisterValue, KEY_NUMf, uErrorInjectKeyNum );
      soc_reg_field_set( pc3hppcTestInfo->nUnit, RC_GLOBAL_DEBUGr, &uRegisterValue, FORCE_KEY_MEM_PARITY_ERRORf, 1 );
      WRITE_RC_GLOBAL_DEBUGr( pc3hppcTestInfo->nUnit, uRegisterValue );

      sal_sleep(2);

      soc_reg_field_set( pc3hppcTestInfo->nUnit, RC_GLOBAL_DEBUGr, &uRegisterValue, FORCE_KEY_MEM_PARITY_ERRORf, 0 );
      WRITE_RC_GLOBAL_DEBUGr( pc3hppcTestInfo->nUnit, uRegisterValue );

      for ( uFlowID = 0; uFlowID < g_nFlowTableSize; ++uFlowID ) {
        c3hppc_cop_coherent_table_read_write( pc3hppcTestInfo->nUnit, 0, 0, uFlowID, 0, OcmEntry.uData );
        if ( (uFlowID % 128) == uErrorInjectKeyNum ) {
          if ( OcmEntry.uData[0] & 0x80000000 ) {
            cli_out("<c3hppc_rce_test1__run> -- Injected error indication seen on FlowID[%d]! \n", uFlowID );
            OcmEntry.uData[0] &= 0x7fffffff;
            c3hppc_cop_coherent_table_read_write( pc3hppcTestInfo->nUnit, 0, 0, uFlowID, 1, OcmEntry.uData );
          } else {
            cli_out("<c3hppc_rce_test1__run> -- Injected error indication NOT seen on FlowID[%d]! \n", uFlowID );
            pc3hppcTestInfo->nTestStatus = TEST_FAIL;
          }
        } else if ( OcmEntry.uData[0] & 0x80000000 ) {
          cli_out("<c3hppc_rce_test1__run> -- UNEXPECTED ERROR indication seen on FlowID[%d]! \n", uFlowID );
          pc3hppcTestInfo->nTestStatus = TEST_FAIL;
        }
      }

      c3hppc_lrp_write_shared_register( pc3hppcTestInfo->nUnit, 0, 0 );

      uRegisterValue = 0;
      soc_reg_field_set( pc3hppcTestInfo->nUnit, RC_GLOBAL_ERRORr, &uRegisterValue, KEY_MEM_PARITY_ERRORf, 1 );
      WRITE_RC_GLOBAL_ERRORr( pc3hppcTestInfo->nUnit, uRegisterValue );

      pc3hppcTestInfo->nErrorInject = 0;
    }

    c3hppc_rce_read_results_counters( pc3hppcTestInfo->nUnit );
  }

  cli_out("\nDetected LRP \"offline\" event\n");


  return 0;
}

int
c3hppc_rce_test1__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
#if !defined(COMPILER_HAS_DOUBLE) || !defined(COMPILER_HAS_LONGLONG)

  cli_out("\nTHIS TEST DOES NOT SUPPORT THIS PLATFORM/OS!!!!! \n");
  return SOC_E_UNAVAIL;

#else

  int nErrorCnt;
  uint64 *pMatchPktCount, *pNoMatchPktCount, *pSkippedLookupPktCount;
  uint64 *pMisMatchPktCount, *pLaunchedLookupPktCount, *pPreSwitchNoMatchPktCount;
  uint64 uuExpectPktCount;
  c3hppc_cmu_segment_info_t *pCmuSegment0Info, *pCmuSegment1Info;
  uint32 uFlowID, uReg, uExpect;
  uint64 uuTotalLaunchedPktCount, uuTotalPreSwitchNoMatchPktCount, uuTotalSkippedLookupPktCount; 
  uint64 uuTotalNoMatchPktCount; 
  int nCounterId;
  uint64 uuSum, uuTmp;
  c3hppc_64b_ocm_entry_template_t uOcmEntryExpect;
  int nOcmPort;




  c3hppc_rce_read_results_counters( pc3hppcTestInfo->nUnit );

  /* Check FlowTable contents at end of test */
  uOcmEntryExpect.uData[1] = g_pFlowTable[0].uData[1];
  uOcmEntryExpect.uData[0] = g_pFlowTable[0].uData[0];

  nErrorCnt = 0;
  nOcmPort = c3hppc_ocm_map_cop2ocm_port(0);
  c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, 0, 0, (g_nFlowTableSize-1), 0, g_pFlowTable[0].uData );
  for ( uFlowID = 0; uFlowID < g_nFlowTableSize; ++uFlowID ) {
    if ( g_pFlowTable[uFlowID].uData[1] != uOcmEntryExpect.uData[1] ||
         g_pFlowTable[uFlowID].uData[0] != uOcmEntryExpect.uData[0] ) {
      if ( nErrorCnt < 10 ) {
        cli_out("FlowTable entry[%d] --> Actual: 0x%08x_%08x   Expect: 0x%08x_%08x\n",
                uFlowID, g_pFlowTable[uFlowID].uData[1], g_pFlowTable[uFlowID].uData[0],
                uOcmEntryExpect.uData[1], uOcmEntryExpect.uData[0] );
      }
      ++nErrorCnt;
    }
  }

  if ( g_nNumberOfPrograms == 1 ) {

    nCounterId = 0;
    COMPILER_64_ZERO(uuTotalLaunchedPktCount);
    COMPILER_64_ZERO(uuTotalPreSwitchNoMatchPktCount);
    COMPILER_64_ZERO(uuTotalSkippedLookupPktCount);
    COMPILER_64_ZERO(uuTotalNoMatchPktCount);
    pCmuSegment0Info = pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo + 0;
    pCmuSegment1Info = pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo + 1;
    for ( uFlowID = 0; uFlowID < C3HPPC_TEST_SIMPLEX_FLOW_TABLE_SIZE; ++uFlowID ) {
      pMatchPktCount = pCmuSegment0Info->pSegmentPciBase + (2 * uFlowID) + 1;
      pSkippedLookupPktCount = pCmuSegment0Info->pSegmentPciBase + (2 * (C3HPPC_TEST_SIMPLEX_FLOW_TABLE_SIZE + uFlowID)) + 1;
      pNoMatchPktCount = pCmuSegment0Info->pSegmentPciBase + (4 * C3HPPC_TEST_SIMPLEX_FLOW_TABLE_SIZE) + (2 * uFlowID) + 1;
      pMisMatchPktCount = pCmuSegment1Info->pSegmentPciBase + (2 * uFlowID) + 1;
      pLaunchedLookupPktCount = pCmuSegment1Info->pSegmentPciBase + (2 * (C3HPPC_TEST_SIMPLEX_FLOW_TABLE_SIZE + uFlowID)) + 1;
      pPreSwitchNoMatchPktCount = pCmuSegment1Info->pSegmentPciBase + (4 * C3HPPC_TEST_SIMPLEX_FLOW_TABLE_SIZE) + (2 * uFlowID) + 1;

      COMPILER_64_SET(uuExpectPktCount, COMPILER_64_HI(pc3hppcTestInfo->uuIterations), COMPILER_64_LO(pc3hppcTestInfo->uuIterations));
      COMPILER_64_ADD_64(uuTotalLaunchedPktCount, *pLaunchedLookupPktCount);
      COMPILER_64_ADD_64(uuTotalPreSwitchNoMatchPktCount, *pPreSwitchNoMatchPktCount);
      COMPILER_64_ADD_64(uuTotalSkippedLookupPktCount, *pSkippedLookupPktCount);
      COMPILER_64_ADD_64(uuTotalNoMatchPktCount, *pNoMatchPktCount);

      COMPILER_64_SET(uuTmp, COMPILER_64_HI(*pMatchPktCount), COMPILER_64_LO(*pMatchPktCount));
      COMPILER_64_ADD_64(uuTmp, *pNoMatchPktCount);
      COMPILER_64_ADD_64(uuTmp, *pSkippedLookupPktCount);
      if ( COMPILER_64_NE(uuTmp, uuExpectPktCount) ) {
        if ( nErrorCnt < 128 ) {
          cli_out("<c3hppc_rce_test1__done> -- Packet count MISCOMPARE on FlowID[%d]  Actual: %lld   Expect: %lld \n",
                  uFlowID, uuTmp, uuExpectPktCount );
        }
        ++nErrorCnt;
      }
      if ( !COMPILER_64_IS_ZERO(*pMisMatchPktCount) ) {
        if ( nErrorCnt < 128 ) {
          cli_out("<c3hppc_rce_test1__done> -- Lookup MISCOMPARES on FlowID[%d] --> %lld \n",
                  uFlowID, *pMisMatchPktCount );
        }
        ++nErrorCnt;
      }

      if ( (((uFlowID % 128) == 0 ) && (nCounterId < C3HPPC_RCE_RESULTS_COUNTER_NUM/2))) {
        uuSum = c3hppc_rce_get_results_counter(nCounterId);
        uuTmp = c3hppc_rce_get_results_counter(nCounterId+8);
        COMPILER_64_ADD_64(uuSum, uuTmp);
        if ( COMPILER_64_NE(*pMatchPktCount, uuSum) ) {
          COMPILER_64_SET(uuTmp, COMPILER_64_HI(uuSum), COMPILER_64_LO(uuSum));
          COMPILER_64_SUB_64(uuTmp, *pMatchPktCount);
          COMPILER_64_SUB_32(uuTmp, 1);
          if ( !(COMPILER_64_IS_ZERO(uuTmp) && 
                 (nCounterId == 0 || nCounterId == 5 || nCounterId == 2 || nCounterId == 7 || nCounterId == 4)) ) {  
            cli_out("<c3hppc_rce_test1__done> -- RC_RESULTS_RULE_CNT MISCOMPARES on CounterID[%d] -->  Actual: %lld  Expect: %lld \n",
                    nCounterId, uuSum, *pMatchPktCount );
            ++nErrorCnt;
          }
        }
        ++nCounterId;
      }
    }

    COMPILER_64_SET(uuTmp, COMPILER_64_HI(uuTotalLaunchedPktCount), COMPILER_64_LO(uuTotalLaunchedPktCount));
    COMPILER_64_SUB_64(uuTmp, uuTotalPreSwitchNoMatchPktCount);
    uExpect = COMPILER_64_LO(uuTmp);

    READ_RC_RESULTS_MATCH_CNTr( pc3hppcTestInfo->nUnit, &uReg );
    if ( uReg != uExpect ) {
      cli_out("<c3hppc_rce_test1__done> -- RC_RESULTS_MATCH_CNT count MISCOMPARE   Actual: %d   Expect: %d \n",
              uReg, uExpect );
      ++nErrorCnt;
    }

    cli_out("\n\n<c3hppc_rce_test1__done> -- RC_RESULTS_MATCH_CNT --> %d\n", uReg);
    cli_out("<c3hppc_rce_test1__done> -- LAUNCHED_LOOKUP_COUNT --> %lld\n", uuTotalLaunchedPktCount);
    cli_out("<c3hppc_rce_test1__done> -- SKIPPED_LOOKUP_COUNT --> %lld\n", uuTotalSkippedLookupPktCount);
    cli_out("<c3hppc_rce_test1__done> -- NO_MATCH_LOOKUP_COUNT --> %lld\n", uuTotalNoMatchPktCount);
    cli_out("<c3hppc_rce_test1__done> -- PRE_SWITCH_NO_MATCH_LOOKUP_COUNT --> %lld\n\n", uuTotalPreSwitchNoMatchPktCount);

    for ( nCounterId = 0; nCounterId < C3HPPC_RCE_RESULTS_COUNTER_NUM; ++nCounterId ) {
      cli_out("<c3hppc_rce_test1__done> -- Results rule [0x%05x] counter value --> %lld\n", 
              g_auResultsRuleCountConfigLo[nCounterId], c3hppc_rce_get_results_counter(nCounterId) );
      uuTmp = c3hppc_rce_get_results_counter(nCounterId);
      if ( COMPILER_64_LO(pc3hppcTestInfo->uuIterations) >= 10000 && 
           nCounterId < 6 && COMPILER_64_NE(g_auuResultsRuleCountersSnapShot[nCounterId], uuTmp) ) {
        cli_out("<c3hppc_rce_test1__done> -- RC_RESULTS_RULE_CNT count snapshot MISCOMPARE -->  Actual: %lld   Expect: %lld \n",
                c3hppc_rce_get_results_counter(nCounterId), g_auuResultsRuleCountersSnapShot[nCounterId] );
        ++nErrorCnt;
      }
    }

  } /* if ( g_nNumberOfPrograms == 1 ) { */

  if ( nErrorCnt ) {
    pc3hppcTestInfo->nTestStatus = TEST_FAIL;
    cli_out("\n<c3hppc_rce_test1__done> -- Total error count --> %d\n", nErrorCnt );
  }

  soc_cm_sfree(pc3hppcTestInfo->nUnit, g_pFlowTable);

  return 0;
#endif
}

#endif /* #ifdef BCM_CALADAN3_SUPPORT */
