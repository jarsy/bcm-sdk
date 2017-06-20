/* $Id: c3hppc_cop_test2.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/defs.h>


#ifdef BCM_CALADAN3_SUPPORT

#include "../c3hppc_test.h"
 
#define C3HPPC_COP_TEST2__SEGMENT_NUM  2
#define C3HPPC_COP_TEST2__STREAM_NUM   1

#define C3HPPC_COP_TEST2__TASK_NUM  2

static c3hppc_64b_ocm_entry_template_t *g_pFlowTables[C3HPPC_COP_TEST2__TASK_NUM];
static int g_nFlowTableSize;


static int nCopSegment;

int
c3hppc_cop_test2__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  int rc, nCopInstance;
  int nOcmPort, nSegment, nStartingPhysicalBlock1, nStartingPhysicalBlock0;
  c3hppc_cmu_segment_info_t *pCmuSegmentInfo;
  c3hppc_cop_segment_info_t *pCopSegmentInfo;
  c3hppc_cop_profile_info_t *pCopProfileInfo;
  c3hppc_64b_ocm_entry_template_t *pOcmBlock;
  int nDmaBlockSize, nMeter;
  uint32 uBktE, uBktC; 

  c3hppc_populate_with_defaults( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );

  strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "cop_test2.oasm");
  nCopSegment = 0;

  nStartingPhysicalBlock0 = 16;
  nStartingPhysicalBlock1 = 16;
  for ( nSegment = 0; nSegment < C3HPPC_COP_INSTANCE_NUM; ++nSegment ) {
    pCmuSegmentInfo = &(pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo[nSegment]);
    pCmuSegmentInfo->bValid = 1;
    pCmuSegmentInfo->uSegment = (uint32) nSegment;
    pCmuSegmentInfo->uSegmentOcmBase = 0;
    pCmuSegmentInfo->uSegmentType = C3HPPC_CMU_SEGMENT_TYPE__TURBO_32b; 
    pCmuSegmentInfo->uSegmentLimit = pc3hppcTestInfo->nCounterNum - 1;
    pCmuSegmentInfo->uSegmentPort = nSegment;
    pCmuSegmentInfo->nStartingPhysicalBlock = ( nSegment ) ? nStartingPhysicalBlock1 :
                                                             nStartingPhysicalBlock0;
  }

  nStartingPhysicalBlock0 = 32;
  nStartingPhysicalBlock1 = 32;
  for ( nCopInstance = 0; nCopInstance < C3HPPC_COP_INSTANCE_NUM; ++nCopInstance ) {
    pCopSegmentInfo = &(pc3hppcTestInfo->BringUpControl.aCopSegmentInfo[nCopInstance][nCopSegment]);
    pCopSegmentInfo->bValid = 1;
    pCopSegmentInfo->uProfile = 0;
    pCopSegmentInfo->uSegmentBase = 0;
    pCopSegmentInfo->uSegmentLimit = pc3hppcTestInfo->nCounterNum - 1;
    pCopSegmentInfo->uSegmentOcmLimit = pCopSegmentInfo->uSegmentLimit;
    pCopSegmentInfo->uSegmentType = C3HPPC_COP_SEGMENT_TYPE__METER;
    pCopSegmentInfo->uSegmentTransferSize = C3HPPC_DATUM_SIZE_QUADWORD;
    pCopSegmentInfo->nStartingPhysicalBlock = ( nCopInstance ) ? nStartingPhysicalBlock1++ :
                                                                 nStartingPhysicalBlock0++;
    pCopSegmentInfo->uRefreshVisitInterval = 0;
    COMPILER_64_SET(pCopSegmentInfo->uuRefreshVisitPeriod, 0, 1 << 20);

    pCopProfileInfo = &(pc3hppcTestInfo->BringUpControl.aCopProfileInfo[nCopInstance][pCopSegmentInfo->uProfile]);
    if ( !pCopProfileInfo->bValid ) { 
      if ( pCopSegmentInfo->uProfile == 0 ) {
        pCopProfileInfo->bValid = 1;
        pCopProfileInfo->uCBSinBytes = 1250000;
        pCopProfileInfo->uEBSinBytes = 1250000;
        pCopProfileInfo->uCIRinKbps = 10000000;
        pCopProfileInfo->uEIRinKbps = 10000000;
        pCopProfileInfo->bDropOnRed = 1;
        pCopProfileInfo->bRFC2698 = 1;
      }
    }
  }

  rc = c3hppc_bringup( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );
  if ( rc ) return 1;

  uBktE = 0x7ffffff;
  uBktC = 0x7ffffff;

  for ( nCopInstance = 0; nCopInstance < C3HPPC_COP_INSTANCE_NUM; ++nCopInstance ) {

    /* Initialize meter state */
    pCopSegmentInfo = &(pc3hppcTestInfo->BringUpControl.aCopSegmentInfo[nCopInstance][nCopSegment]);
    nDmaBlockSize = pCopSegmentInfo->uSegmentLimit + 1;
    pOcmBlock = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                                                  nDmaBlockSize * sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                  "ocm_block");
    for ( nMeter = 0; nMeter <= (int) pCopSegmentInfo->uSegmentLimit; ++nMeter ) {
      c3hppc_cop_format_meterstate_in_ocm_entry( &(pOcmBlock[nMeter]),
                                                   pCopSegmentInfo->uProfile, uBktE, uBktC );
    } 
    nOcmPort = c3hppc_ocm_map_cop2ocm_port(nCopInstance);
    c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, 0, 0, (nDmaBlockSize-1), 1, pOcmBlock->uData );
    soc_cm_sfree(pc3hppcTestInfo->nUnit,pOcmBlock);

    c3hppc_cop_meter_monitor_setup( pc3hppcTestInfo->nUnit, nCopInstance, 0, nCopSegment, 0 );

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
c3hppc_cop_test2__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{

  int nOcmPort;
  int nIndex, nTask;
  uint32 uReg, uContexts;
  uint8 bResponseCheckingMode;

  g_nFlowTableSize = 1024;
  for ( nTask = 0; nTask < C3HPPC_COP_TEST2__TASK_NUM; ++nTask ) {
    g_pFlowTables[nTask] = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                                                             g_nFlowTableSize * sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                             "flow_table");
    sal_memset( g_pFlowTables[nTask], 0, (g_nFlowTableSize * sizeof(c3hppc_64b_ocm_entry_template_t)) );
  }
  /****************************************************************************************************************************
   * A FlowTable entry contains the parameter to select the stream of CMGR operations.
   *               
   * 19:0  --> Counter ID mask
   * 20:20 --> Do response checking
   *****************************************************************************************************************************/
  for ( nTask = 0; nTask < C3HPPC_COP_TEST2__TASK_NUM; ++nTask ) {
    bResponseCheckingMode = sal_rand() % 2;
/* Coverity
bResponseCheckingMode = 0;
*/
    for ( nIndex = 0; nIndex < g_nFlowTableSize; ++nIndex ) {
      if ( bResponseCheckingMode ) {
        g_pFlowTables[nTask][nIndex].uData[0] = ((pc3hppcTestInfo->nCounterNum >> 10) - 1);
        g_pFlowTables[nTask][nIndex].uData[0] |= 0x100000;
      } else {
        g_pFlowTables[nTask][nIndex].uData[0] = (pc3hppcTestInfo->nCounterNum - 1);
      }
    }
    nOcmPort = c3hppc_ocm_map_lrp2ocm_port(nTask);
    c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, C3HPPC_DATUM_SIZE_QUADWORD, 
                               0, (g_nFlowTableSize-1), 1, g_pFlowTables[nTask][0].uData );
  }

  READ_LRA_CONFIG1r( pc3hppcTestInfo->nUnit, &uReg );
  uContexts = soc_reg_field_get( pc3hppcTestInfo->nUnit, LRA_CONFIG1r, uReg, CONTEXTSf );
  COMPILER_64_SET(pc3hppcTestInfo->uuEpochCount,
                  COMPILER_64_HI(pc3hppcTestInfo->uuIterations), COMPILER_64_LO(pc3hppcTestInfo->uuIterations));
  COMPILER_64_UMUL_32(pc3hppcTestInfo->uuEpochCount, C3HPPC_COP_TEST2__STREAM_NUM * uContexts);
  { uint64 uuTmp;
    COMPILER_64_SET(uuTmp, COMPILER_64_HI(pc3hppcTestInfo->uuEpochCount), COMPILER_64_LO(pc3hppcTestInfo->uuEpochCount));
    COMPILER_64_ADD_64(uuTmp, pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition);
    c3hppc_lrp_start_control( pc3hppcTestInfo->nUnit, uuTmp);
  }

  while ( !c3hppc_is_test_done( pc3hppcTestInfo->nUnit ) ) {
    sal_sleep(5);
    assert( !c3hppc_cmu_display_error_state(pc3hppcTestInfo->nUnit) );
  }


  return 0;
}

int
c3hppc_cop_test2__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
#if !defined(COMPILER_HAS_DOUBLE) || !defined(COMPILER_HAS_LONGLONG)

  cli_out("\nTHIS TEST DOES NOT SUPPORT THIS PLATFORM/OS!!!!! \n");
  return SOC_E_UNAVAIL;

#else

  int nErrorCnt, nTask;
  uint64 *pBytCounter;
  int nSegment;
  uint32 uCounterId;
  uint64 uuExpectBytCount;
  uint64 uuCopActualBytCount;
  c3hppc_cmu_segment_info_t *pCmuSegmentInfo;
  uint32 uReg;
  c3hppc_64b_ocm_entry_template_t *pOcmBlock;
  int nDmaBlockSize;
  int nCopInstance, nOcmPort;
  uint64 uuPktCount; /* uuByteCount */
  int nIndex;


  /* Check OCM contents at end of test */
  nDmaBlockSize = pc3hppcTestInfo->nCounterNum;
  pOcmBlock = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                                                nDmaBlockSize * sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                "ocm_block");


  nErrorCnt = 0;
  for ( nCopInstance = 0; nCopInstance < C3HPPC_COP_INSTANCE_NUM; ++nCopInstance ) {

    c3hppc_cop_meter_monitor_dump_memory( pc3hppcTestInfo->nUnit, nCopInstance );

    nSegment = nCopInstance;
    pCmuSegmentInfo = pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo + nSegment;
    nOcmPort = c3hppc_ocm_map_cop2ocm_port(nCopInstance);
    c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, 0, 0, (nDmaBlockSize-1), 0, pOcmBlock->uData );
    for ( uCounterId = 0; uCounterId < pc3hppcTestInfo->nCounterNum; ++uCounterId ) {
      pBytCounter = pCmuSegmentInfo->pSegmentPciBase + (2 * uCounterId);
      uuExpectBytCount = *pBytCounter;
      COMPILER_64_SET(uuCopActualBytCount, pOcmBlock[uCounterId].uData[1], pOcmBlock[uCounterId].uData[0]);
      if ( COMPILER_64_NE(uuCopActualBytCount, uuExpectBytCount) ) {
        if ( nErrorCnt < 128 ) {
/*
          cli_out("<c3hppc_cop_test2__done> -- Packet byte MISCOMPARE for CMU_Segment[%d] CountID[%d]  Actual: 0x%llx   Expect: 0x%llx \n",
                  nSegment, uCounterId, uuCopActualBytCount, uuExpectBytCount );
*/
        }
        ++nErrorCnt;
      }
    }

    for ( nIndex = 0; nIndex < C3HPPC_COP_METER_MONITOR_COUNTERS_PER_GROUP; ++nIndex ) {
      uuPktCount = c3hppc_cop_meter_monitor_get_packet_count( nIndex, nCopInstance );
      /* uuByteCount = c3hppc_cop_meter_monitor_get_byte_count( nIndex, nCopInstance );  */
      cli_out("<c3hppc_cop_test2__done> -- COP[%d] Meter[0] Counter[%d] packet count --> %lld \n", nCopInstance, nIndex, uuPktCount );  
    }


  }

  if ( nErrorCnt ) {
    pc3hppcTestInfo->nTestStatus = TEST_FAIL;
    cli_out("\n<c3hppc_cop_test2__done> -- Total error count --> %d\n", nErrorCnt );
  }

  READ_CM_EF_MAX_DEPTHr( pc3hppcTestInfo->nUnit, &uReg );
  cli_out("\n<c3hppc_cop_test2__done> -- CM_EF_MAX_DEPTH --> %d\n", uReg );

  for ( nTask = 0; nTask < C3HPPC_COP_TEST2__TASK_NUM; ++nTask ) {
    soc_cm_sfree(pc3hppcTestInfo->nUnit, g_pFlowTables[nTask]);
  }
  soc_cm_sfree(pc3hppcTestInfo->nUnit,pOcmBlock);

  return 0;
#endif
}

#endif /* #ifdef BCM_CALADAN3_SUPPORT */
