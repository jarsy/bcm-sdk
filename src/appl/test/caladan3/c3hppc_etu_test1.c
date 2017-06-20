/* $Id: c3hppc_etu_test1.c,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/defs.h>


#ifdef BCM_CALADAN3_SUPPORT

#include "../c3hppc_test.h"
 
#define C3HPPC_ETU_TEST1__LOGICAL_TABLE                       (0)
#define C3HPPC_ETU_TEST1__ETU_SEARCH_PROGRAM                  (0)
#define C3HPPC_ETU_TEST1__ETU_NOP_PROGRAM                     (10)
#define C3HPPC_ETU_TEST1__LRP_KEY_PROGRAM_NUMBER              (0)

#define C3HPPC_ETU_TEST1__TABLE_STATE_CHANGE                  (4)

static c3hppc_64b_ocm_entry_template_t *g_pFlowTable;
static int g_nFlowTableSize;
static int g_nDataBaseLayout;
static uint8 g_bPerformanceTest;


int
c3hppc_etu_test1__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  int rc, nCopInstance;
  uint32 uResultsTimer;
  uint16 dev_id;
  uint8 rev_id;
  soc_cm_get_id( pc3hppcTestInfo->nUnit, &dev_id, &rev_id);

  /* From c3hppc_etu.h --> C3HPPC_ETU_LOOKUP__80(0), C3HPPC_ETU_LOOKUP__160(1), C3HPPC_ETU_LOOKUP__320(2), C3HPPC_ETU_LOOKUP__640(3)
                           C3HPPC_ETU_LOOKUP__4x80(4)
  
             Layout            Key Capacity
     C3HPPC_ETU_LOOKUP__80        512K 
     C3HPPC_ETU_LOOKUP__160       256K 
     C3HPPC_ETU_LOOKUP__320       128K 
     C3HPPC_ETU_LOOKUP__640       64K   -- Going to require new ucode to deal with latency -- 071912 -- Priority 2 test 
     C3HPPC_ETU_LOOKUP__4x80      128K 
  */ 
  g_nDataBaseLayout = pc3hppcTestInfo->nSetupOptions;
  g_bPerformanceTest = pc3hppcTestInfo->bPerformanceTest;; 

  c3hppc_populate_with_defaults( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );
  pc3hppcTestInfo->BringUpControl.uEtuBringUp = 1;


  pc3hppcTestInfo->BringUpControl.bLrpLoaderEnable = 1;

  COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,2);

  uResultsTimer = 0;
  if ( g_nDataBaseLayout == C3HPPC_ETU_LOOKUP__160 ) {
    if ( pc3hppcTestInfo->nNL11KFreq == 400 )
      strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "etu_test1_key160_400mhz.oasm");
    else
      strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "etu_test1_key160.oasm");
    uResultsTimer = 0;
  } else if ( g_nDataBaseLayout == C3HPPC_ETU_LOOKUP__320 ) {
    if ( pc3hppcTestInfo->nNL11KFreq == 400 )
      strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "etu_test1_key320_400mhz.oasm");
    else
      strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "etu_test1_key320.oasm");
  } else if ( g_nDataBaseLayout == C3HPPC_ETU_LOOKUP__640 ) {
    COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,4);
    uResultsTimer = 384;
    strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "etu_test1_key640.oasm");
  } else {
    if ( g_bPerformanceTest ) {
      uResultsTimer = 0xffff;
      if ( rev_id == BCM88030_A0_REV_ID )
        strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "etu_test1_key80_perf_a0.oasm");
      else
        strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "etu_test1_key80_perf.oasm");
    } else if ( pc3hppcTestInfo->nNL11KFreq == 400 )
      strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "etu_test1_key80_400mhz.oasm");
    else
      strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "etu_test1_key80.oasm");
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

  c3hppc_lrp_set_results_timer( pc3hppcTestInfo->nUnit, uResultsTimer );

  return 0;
}

int
c3hppc_etu_test1__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{

  int nOcmPort, nCopInstance;
  uint32 uReg, uContexts;
/*
  uint64 uuDW0, uuDW1;
*/
  c3hppc_etu_80b_data_t *pKeyData;
  c3hppc_etu_80b_data_t *pKeyMask;
  int nKey;
  int nTimeOut, nIndex;
  uint32 uKeySizeIn80bSegments, uSegment;






  g_nFlowTableSize = 1024;
  g_pFlowTable = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                                                   g_nFlowTableSize * sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                   "flow_table");
  sal_memset( g_pFlowTable, 0, (g_nFlowTableSize * sizeof(c3hppc_64b_ocm_entry_template_t)) );
  /****************************************************************************************************************************
   * A FlowTable entry contains the parameter to select the stream of CMGR operations.
   *
   * 0:0   --> Allow result misses due to key state transitions
   * 7:4   --> Key size result shift
   * 11:8  --> Parallel Search Number
   *****************************************************************************************************************************/
  for ( nIndex = 0; nIndex < g_nFlowTableSize; ++nIndex ) {
    if ( pc3hppcTestInfo->nHostActivityControl == C3HPPC_ETU_TEST1__TABLE_STATE_CHANGE ) {
      g_pFlowTable[nIndex].uData[0] |= 0x00000001;
    }
    g_pFlowTable[nIndex].uData[0] |= (g_nDataBaseLayout & 3) << 4;
    g_pFlowTable[nIndex].uData[0] |= (( g_nDataBaseLayout == C3HPPC_ETU_LOOKUP__4x80 ) ? 4 : 1) << 8;
  }
  for ( nCopInstance = 0; nCopInstance < C3HPPC_COP_INSTANCE_NUM; ++nCopInstance ) {
    nOcmPort = c3hppc_ocm_map_cop2ocm_port( nCopInstance );
    c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, C3HPPC_DATUM_SIZE_QUADWORD,
                               0, (g_nFlowTableSize-1), 1, g_pFlowTable[0].uData );
  }



  /****************************************************************************************************************************
   * This is here for when this test is only being run to bring up the NL11K emulation board.
   *****************************************************************************************************************************/
  if ( SAL_BOOT_QUICKTURN && 
       (COMPILER_64_LO(pc3hppcTestInfo->uuIterations) == 1) && 
       (COMPILER_64_HI(pc3hppcTestInfo->uuIterations) == 0) ) return 0;


  
  c3hppc_etu_tcam_table_layout_setup( pc3hppcTestInfo->nUnit, C3HPPC_ETU_TEST1__LOGICAL_TABLE,
                                      g_nDataBaseLayout, pc3hppcTestInfo->nMaxKey );

  c3hppc_lrp_setup_etu_program( pc3hppcTestInfo->nUnit, C3HPPC_ETU_TEST1__LRP_KEY_PROGRAM_NUMBER,
                                                        C3HPPC_ETU_TEST1__ETU_SEARCH_PROGRAM );

  c3hppc_etu_setup_search_program( pc3hppcTestInfo->nUnit, C3HPPC_ETU_TEST1__ETU_SEARCH_PROGRAM,
                                   C3HPPC_ETU_TEST1__LOGICAL_TABLE, g_nDataBaseLayout );

  uKeySizeIn80bSegments = c3hppc_etu_get_tcam_table_key_size( C3HPPC_ETU_TEST1__LOGICAL_TABLE ); 
  pKeyData = (c3hppc_etu_80b_data_t *) sal_alloc( (pc3hppcTestInfo->nMaxKey * uKeySizeIn80bSegments * sizeof(c3hppc_etu_80b_data_t)),
                                                  "ETU Key Data");
  pKeyMask = (c3hppc_etu_80b_data_t *) sal_alloc( (pc3hppcTestInfo->nMaxKey * uKeySizeIn80bSegments * sizeof(c3hppc_etu_80b_data_t)),
                                                  "ETU Key Mask");
  sal_memset( pKeyData, 0x00, (pc3hppcTestInfo->nMaxKey * uKeySizeIn80bSegments * sizeof(c3hppc_etu_80b_data_t)) );
  sal_memset( pKeyMask, 0x00, (pc3hppcTestInfo->nMaxKey * uKeySizeIn80bSegments * sizeof(c3hppc_etu_80b_data_t)) );

  for ( nKey = 0; nKey < pc3hppcTestInfo->nMaxKey; ++nKey ) {
    for ( uSegment = 0; uSegment < uKeySizeIn80bSegments; ++uSegment ) {
      nIndex = (nKey * uKeySizeIn80bSegments) + uSegment;
      if ( uSegment == 0 ) {
        COMPILER_64_SET(pKeyData[nIndex].Words[0], 0, nKey);
        COMPILER_64_ZERO(pKeyData[nIndex].Words[1]);
        COMPILER_64_ZERO(pKeyMask[nIndex].Words[0]);
        COMPILER_64_ZERO(pKeyMask[nIndex].Words[1]);
      } else if ( (uSegment+1) == uKeySizeIn80bSegments ) {
        /* MSB of key is set to '1' for keys greater than 1 segment. */
        COMPILER_64_SET(pKeyData[nIndex].Words[1], 0x00000000,0x00008000);
      }
    } 
  } 

  c3hppc_etu_key_insert( C3HPPC_ETU_TEST1__LOGICAL_TABLE, 0x00000, pc3hppcTestInfo->nMaxKey, pKeyData, pKeyMask, 0 );
  if ( g_nDataBaseLayout == C3HPPC_ETU_LOOKUP__4x80 ) {
    c3hppc_etu_key_insert( C3HPPC_ETU_TEST1__LOGICAL_TABLE, 0x20000, pc3hppcTestInfo->nMaxKey, pKeyData, pKeyMask, 0 );
    c3hppc_etu_key_insert( C3HPPC_ETU_TEST1__LOGICAL_TABLE, 0x40000, pc3hppcTestInfo->nMaxKey, pKeyData, pKeyMask, 0 );
    c3hppc_etu_key_insert( C3HPPC_ETU_TEST1__LOGICAL_TABLE, 0x60000, pc3hppcTestInfo->nMaxKey, pKeyData, pKeyMask, 0 );
  }


  /****************************************************************************************************************************
   * Wait for Updater operations to complete before advancing to enable traffic.
   *****************************************************************************************************************************/
  nTimeOut = 5;
  if ( c3hppc_test__wait_for_updaters_to_be_idle(pc3hppcTestInfo->nUnit,nTimeOut) == 0 ) {

    cli_out("ERROR:  Initial setup Updater TIMEOUT failure!\n");
    pc3hppcTestInfo->nTestStatus = TEST_FAIL;

  } else {

    c3hppc_lrp_setup_ring_wheel( pc3hppcTestInfo->nUnit, 0, 0, pc3hppcTestInfo->nMaxKey, 0 );

    /****************************************************************************************************************************
     * Enable lookups ...
     *****************************************************************************************************************************/
    if ( pc3hppcTestInfo->BringUpControl.bLrpLoaderEnable ) {
      c3hppc_lrp_setup_pseudo_traffic_bubbles( pc3hppcTestInfo->nUnit, 0, 0, (g_bPerformanceTest ? 0 : C3HPPC_LRP__TASK_NUM) );
    }

    READ_LRA_CONFIG1r( pc3hppcTestInfo->nUnit, &uReg );
    uContexts = soc_reg_field_get( pc3hppcTestInfo->nUnit, LRA_CONFIG1r, uReg, CONTEXTSf );
    COMPILER_64_SET(pc3hppcTestInfo->uuEpochCount, 
                    COMPILER_64_HI(pc3hppcTestInfo->uuIterations), COMPILER_64_LO(pc3hppcTestInfo->uuIterations));
    COMPILER_64_UMUL_32(pc3hppcTestInfo->uuEpochCount, uContexts);
    { uint64 uuTmp;
      COMPILER_64_SET(uuTmp, COMPILER_64_HI(pc3hppcTestInfo->uuEpochCount), COMPILER_64_LO(pc3hppcTestInfo->uuEpochCount));
      COMPILER_64_ADD_64(uuTmp, pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition);
      c3hppc_lrp_start_control( pc3hppcTestInfo->nUnit, uuTmp);
    }
    c3hppc_etu_adjust_control_path_latency( pc3hppcTestInfo->nUnit );






/*  BUBBLE TABLE DMA bug debug and work-around  JIRA-CA3-2880  To ressurect change "u512nsInterval" in c3hppc_lrp to 0x1000 for 2ms bubbles
                                                               instead of line-rate. 

  c3hppc_lrp_bubble_state_entry_ut BubbleState;
  uint32 auBubbleIntervalTableEntry[1];
  uint64 uuBubbleCntr;
  int nBubble, nIntervalIndex;
  c3hppc_64b_ocm_entry_template_t *pOcmBlock;
  uint32 u512nsInterval, uBubbleCntrState;

    pOcmBlock = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                                                1024 * sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                "ocm_block");
    nOcmPort = c3hppc_ocm_map_bubble2ocm_port(0);
  
    c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, 0, 0, 1023, 0, pOcmBlock->uData );
    for ( nBubble = 0; nBubble < 512; ++nBubble ) {
      cli_out("<c3hppc_etu_test1__run> -- BUBBLE %d   0x%08x 0x%08x \n", nBubble, pOcmBlock[nBubble].uData[1], pOcmBlock[nBubble].uData[0] );
    }
  
    nIntervalIndex = 63;
    u512nsInterval = 0x0000004;
    auBubbleIntervalTableEntry[0] = 0;
    soc_mem_field_set( pc3hppcTestInfo->nUnit, LRB_BUBBLE_INTERVAL_TABLEm, auBubbleIntervalTableEntry, INTERVALf,  &u512nsInterval );
    SOC_IF_ERROR_RETURN(soc_mem_write( pc3hppcTestInfo->nUnit, LRB_BUBBLE_INTERVAL_TABLEm, MEM_BLOCK_ANY,
                                       nIntervalIndex, auBubbleIntervalTableEntry) );
    for ( nBubble = 0; nBubble < 1024; ++nBubble ) {
      BubbleState.value = (uint64) (((uint64)pOcmBlock[nBubble].uData[1]) << 32);
      BubbleState.value |= (uint64) pOcmBlock[nBubble].uData[0];
      BubbleState.bits.IntervalIndex = nIntervalIndex;
      BubbleState.bits.Init = 1;
      pOcmBlock[nBubble].uData[1] = (uint32) (BubbleState.value >> 32);
      pOcmBlock[nBubble].uData[0] = (uint32) (BubbleState.value & COMPILER_64_INIT(0x00000000,0xffffffff));
    }
  
    c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, 0, 0, 1023, 1, pOcmBlock->uData );
    c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, 0, 0, 1023, 0, pOcmBlock->uData );
    for ( nBubble = 0; nBubble < 512; ++nBubble ) {
      cli_out("<c3hppc_etu_test1__run> -- BUBBLE %d   0x%08x 0x%08x \n", nBubble, pOcmBlock[nBubble].uData[1], pOcmBlock[nBubble].uData[0] );
    }
  

    for ( nBubble = 0; nBubble < 1024; ++nBubble ) {
      uReg = ( nBubble & 1 ) ? 0x80008100 : 0x80008000;
      uReg |= nBubble << 16;
      c3hppc_lrp_write_shared_register( pc3hppcTestInfo->nUnit, 17, uReg );
      sal_usleep(1000);
      c3hppc_lrp_setup_host_bubble( pc3hppcTestInfo->nUnit, 1, 0, &uReg );
      sal_usleep(1000);
    }

    nOcmPort = c3hppc_ocm_map_bubble2ocm_port(0);
    c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, 0, 0, 1023, 0, pOcmBlock->uData );
    for ( nBubble = 0; nBubble < 512; ++nBubble ) {
      cli_out("<c3hppc_etu_test1__run> -- BUBBLE %d   0x%08x 0x%08x \n", nBubble, pOcmBlock[nBubble].uData[1], pOcmBlock[nBubble].uData[0] );
    }

    uuBubbleCntr = 0;
    uBubbleCntrState = 0;

    cli_out("\n00000000 Bubbles STARTED at time %d!\n", (int) sal_time() );
    while ( !c3hppc_is_test_done( pc3hppcTestInfo->nUnit ) ) {
      READ_LRA_BUBBLE_CNTr( pc3hppcTestInfo->nUnit, &uReg );
      if ( uBubbleCntrState == 0 && uReg == 0 ) {
        cli_out("\n11111111 Bubbles STOPPED at time %d with %lld bubbles!\n", (int) sal_time(), uuBubbleCntr );
        uBubbleCntrState = 1;
        for ( nBubble = 0; nBubble < 1024; ++nBubble ) {
          uReg = ( nBubble & 1 ) ? 0x8000810b : 0x8000800b;
          uReg |= nBubble << 16;
          c3hppc_lrp_write_shared_register( pc3hppcTestInfo->nUnit, 17, uReg );
          sal_usleep(1000);
          c3hppc_lrp_setup_host_bubble( pc3hppcTestInfo->nUnit, 1, 0, &uReg );
          sal_usleep(1000);
        }
      } else if ( uBubbleCntrState == 1 && uReg != 0 ) {
        cli_out("\n22222222 Bubbles RE-STARTED at time %d with %lld bubbles!\n", (int) sal_time(), uuBubbleCntr );
        uBubbleCntrState = 2;
      } else if ( uBubbleCntrState == 2 && uReg == 0 ) {
        cli_out("\n33333333 Bubbles STOPPED at time %d with %lld bubbles!\n", (int) sal_time(), uuBubbleCntr );
        uBubbleCntrState = 3;
      }

      c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, 0, 0, 1023, 0, pOcmBlock->uData );
      c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, 0, 0, 0, 0, pOcmBlock->uData );

      uuBubbleCntr += (uint64) uReg;
      sal_usleep(1000);
    }
    READ_LRA_BUBBLE_CNTr( pc3hppcTestInfo->nUnit, &uReg );
    uuBubbleCntr += (uint64) uReg;
    cli_out("\n\nINFO:  LRA_BUBBLE_CNT accumulated value --> %lld \n\n", uuBubbleCntr );

    uReg = 0;
    soc_reg_field_set( pc3hppcTestInfo->nUnit, LRB_BUBBLE_TABLE_CONFIGr, &uReg, ENABLEf, 0 );
    soc_reg_field_set( pc3hppcTestInfo->nUnit, LRB_BUBBLE_TABLE_CONFIGr, &uReg, SIZEf, 1024 );
    WRITE_LRB_BUBBLE_TABLE_CONFIGr( pc3hppcTestInfo->nUnit, uReg );

    nOcmPort = c3hppc_ocm_map_bubble2ocm_port(0);
    c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, 0, 0, 1023, 0, pOcmBlock->uData );
    for ( nBubble = 0; nBubble < 1024; ++nBubble ) {
      cli_out("<c3hppc_etu_test1__run> -- BUBBLE %d   0x%08x 0x%08x \n", nBubble, pOcmBlock[nBubble].uData[1], pOcmBlock[nBubble].uData[0] );
    }

    soc_cm_sfree( pc3hppcTestInfo->nUnit, pOcmBlock );
*/




/*
    c3hppc_etu_read_nl11k_register( pc3hppcTestInfo->nUnit, 0, &uuDW1, &uuDW0 ); 
    cli_out("<c3hppc_etu_test1__run> -- Device ID Register -->  DW1: 0x%llx   DW0: 0x%llx \n", uuDW1, uuDW0 );
    c3hppc_etu_read_nl11k_register( pc3hppcTestInfo->nUnit, 1, &uuDW1, &uuDW0 ); 
    cli_out("<c3hppc_etu_test1__run> -- Device Config Register -->  DW1: 0x%llx   DW0: 0x%llx \n", uuDW1, uuDW0 );
    uuDW1 = 0;
    uuDW0 = COMPILER_64_INIT(0x00000000,0x00000080);
    c3hppc_etu_write_nl11k_register( pc3hppcTestInfo->nUnit, 1, uuDW1, uuDW0 ); 
    c3hppc_etu_read_nl11k_register( pc3hppcTestInfo->nUnit, 1, &uuDW1, &uuDW0 ); 
    cli_out("<c3hppc_etu_test1__run> -- Device Config Register -->  DW1: 0x%llx   DW0: 0x%llx \n", uuDW1, uuDW0 );
*/

    if ( pc3hppcTestInfo->nHostActivityControl == C3HPPC_ETU_TEST1__TABLE_STATE_CHANGE ) {
  
      for ( nKey = 0; nKey < pc3hppcTestInfo->nMaxKey; ++nKey ) {
        for ( uSegment = 0; uSegment < uKeySizeIn80bSegments; ++uSegment ) {
          nIndex = (nKey * uKeySizeIn80bSegments) + uSegment;
          if ( (uSegment+1) == uKeySizeIn80bSegments ) {
            uint64 uuTmp = COMPILER_64_INIT(0x00000000,0x00008000);
            COMPILER_64_XOR(pKeyData[nIndex].Words[1], uuTmp);
          }
        } 
      } 
      cli_out("\nINFO:  Doing key deletion ... \n");
      c3hppc_etu_key_insert( C3HPPC_ETU_TEST1__LOGICAL_TABLE, 0, pc3hppcTestInfo->nMaxKey, pKeyData, pKeyMask, 0 );

      if ( c3hppc_test__wait_for_updaters_to_be_idle(pc3hppcTestInfo->nUnit,nTimeOut) == 0 ) {
        cli_out("\nERROR:  Key deletion Updater TIMEOUT failure!\n");
        pc3hppcTestInfo->nTestStatus = TEST_FAIL;
      } else {
        sal_sleep(20);
        c3hppc_lrp_write_shared_register( pc3hppcTestInfo->nUnit, 16, 0 );
        c3hppc_lrp_write_shared_register( pc3hppcTestInfo->nUnit, 32, 0 );
        sal_sleep(10);
        uReg = c3hppc_lrp_read_shared_register( pc3hppcTestInfo->nUnit, 16 );
        if ( uReg ) {
          cli_out("\nERROR:  UNEXPECTED \"matches\" occurring after key deletion.\n");
          pc3hppcTestInfo->nTestStatus = TEST_FAIL;
        }
        uReg = c3hppc_lrp_read_shared_register( pc3hppcTestInfo->nUnit, 32 );
        if ( uReg == 0 ) {
          cli_out("\nERROR:  EXPECTED \"misses\" not occurring after key deletion.\n");
          pc3hppcTestInfo->nTestStatus = TEST_FAIL;
        }
      } 

      if ( pc3hppcTestInfo->nTestStatus != TEST_FAIL ) {
        for ( nKey = 0; nKey < pc3hppcTestInfo->nMaxKey; ++nKey ) {
          for ( uSegment = 0; uSegment < uKeySizeIn80bSegments; ++uSegment ) {
            nIndex = (nKey * uKeySizeIn80bSegments) + uSegment;
            if ( (uSegment+1) == uKeySizeIn80bSegments ) {
              uint64 uuTmp = COMPILER_64_INIT(0x00000000,0x00008000);
              COMPILER_64_XOR(pKeyData[nIndex].Words[1], uuTmp);
            }
          } 
        } 
        cli_out("\nINFO:  Doing key re-insertion ... \n");
        c3hppc_etu_key_insert( C3HPPC_ETU_TEST1__LOGICAL_TABLE, 0, pc3hppcTestInfo->nMaxKey, pKeyData, pKeyMask, 0 );
  
        if ( c3hppc_test__wait_for_updaters_to_be_idle(pc3hppcTestInfo->nUnit,nTimeOut) == 0 ) {
          cli_out("\nERROR:  Key re-insertion Updater TIMEOUT failure!\n");
          pc3hppcTestInfo->nTestStatus = TEST_FAIL;
        } else {
          sal_sleep(20);
          c3hppc_lrp_write_shared_register( pc3hppcTestInfo->nUnit, 16, 0 );
          c3hppc_lrp_write_shared_register( pc3hppcTestInfo->nUnit, 32, 0 );
          sal_sleep(10);
          uReg = c3hppc_lrp_read_shared_register( pc3hppcTestInfo->nUnit, 16 );
          if ( uReg == 0 ) {
            cli_out("\nERROR:  EXPECTED \"matches\" NOT occurring after key re-insertion.\n");
            pc3hppcTestInfo->nTestStatus = TEST_FAIL;
          }
          uReg = c3hppc_lrp_read_shared_register( pc3hppcTestInfo->nUnit, 32 );
          if ( uReg == 1 ) {
            cli_out("\nERROR:  UNEXPECTED \"misses\" occurring after key re-insertion.\n");
            pc3hppcTestInfo->nTestStatus = TEST_FAIL;
          }
          cli_out("\nINFO:  Key re-insertion complete ... \n");
        }
      }
    }

  

    /****************************************************************************************************************************
     * Wait for a "done" indication from the LRP.
     *****************************************************************************************************************************/
    c3hppc_wait_for_test_done( pc3hppcTestInfo->nUnit );

  }


  sal_free( pKeyData );
  sal_free( pKeyMask );


  return 0;
}

int
c3hppc_etu_test1__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
#if !defined(COMPILER_HAS_DOUBLE) || !defined(COMPILER_HAS_LONGLONG)

  cli_out("\nTHIS TEST DOES NOT SUPPORT THIS PLATFORM/OS!!!!! \n");
  return SOC_E_UNAVAIL;

#else

  uint32 uReg, uFifoCnt;
  char sMessage[16];
  c3hppc_cmu_segment_info_t *pCmuSegmentInfo;
  uint64 uuSearchCount; 
  int nInstance;
  double dMsps = 0;

/*
  uint16 uReg0, uReg1, uReg2, uReg3;
  int nESM;
  static int g_anESM_PhyIDs[] = { 0xe1, 0xe5, 0xe9, 0xed, 0xf1, 0xf5 };
  static int g_nNL11K_PhyID = 0x60;
*/

  for ( nInstance = 0; nInstance < (g_bPerformanceTest ? 1 : 2); ++nInstance ) {
    pCmuSegmentInfo = pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo + nInstance;
    uuSearchCount = *(pCmuSegmentInfo->pSegmentPciBase + 1);
    if ( nInstance == 0 ) {
      strcpy( sMessage, "INGRESS" );
    } else {
      strcpy( sMessage, "EGRESS" );
    }
    dMsps = (double) ( uuSearchCount / c3hppc_test__get_test_duration() );
    dMsps /= 1000000.0;
    cli_out("\nINFO:  %s search count --> %lld and rate --> %.3f Msps\n", sMessage, uuSearchCount, dMsps );



/*
  uint64 uuLaunchCount, uuReturnCount; 
    pCmuSegmentInfo = pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo + 0;
    uuReturnCount = *(pCmuSegmentInfo->pSegmentPciBase + 1);
    uuLaunchCount = *(pCmuSegmentInfo->pSegmentPciBase + 3);
    cli_out("\nINFO:  S0 launch count --> %lld return count --> %lld\n", uuLaunchCount, uuReturnCount );
    pCmuSegmentInfo = pc3hppcTestInfo->BringUpControl.aCmuSegmentInfo + 1;
    uuReturnCount = *(pCmuSegmentInfo->pSegmentPciBase + 1);
    uuLaunchCount = *(pCmuSegmentInfo->pSegmentPciBase + 3);
    cli_out("\nINFO:  S1 launch count --> %lld return count --> %lld\n", uuLaunchCount, uuReturnCount );
*/
  }


  for ( nInstance = 0; nInstance < ( g_bPerformanceTest ? 2 : 1 ); ++nInstance ) {
    uReg = c3hppc_lrp_read_shared_register( pc3hppcTestInfo->nUnit, (nInstance ? 16 : 48) );
    if ( uReg ) {
      if ( uReg & 0x08000000 ) strcpy( sMessage, "TSR ERROR" );
      else if ( uReg & 0x02000000 ) strcpy( sMessage, "ETU MISCOMPARE" );
      cli_out("\nERROR:  %s indication received on FlowID --> 0x%03x\n", sMessage, (uReg & 0x3ff) );
    }
  }


  READ_ETU_CP_FIFO_STSr( pc3hppcTestInfo->nUnit, &uReg );
  uFifoCnt = soc_reg_field_get( pc3hppcTestInfo->nUnit, ETU_CP_FIFO_STSr, uReg, CNTf );
  cli_out("\n\nETU_CP_FIFO_STS --> max count is  %d\n", uFifoCnt );

/*
  cli_out("\n\n");
  for ( nESM = 0; nESM < 3; ++nESM ) {
    soc_miimc45_read( pc3hppcTestInfo->nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x80bb, &uReg0 );
    soc_miimc45_read( pc3hppcTestInfo->nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x80cb, &uReg1 );
    soc_miimc45_read( pc3hppcTestInfo->nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x80db, &uReg2 );
    soc_miimc45_read( pc3hppcTestInfo->nUnit, g_anESM_PhyIDs[nESM], 0x1, 0x80eb, &uReg3 );
    cli_out("\nINFO:  ESM%d PF values lane0[0x%04x] lane1[0x%04x] lane2[0x%04x] lane3[0x%04x]\n", nESM, uReg0, uReg1, uReg2, uReg3 );
  }
*/
  cli_out("\n\n");

  /* 
     Our SVK straps the NL11K with a 2X multiplier with a 156.25MHz crystal.  The part should really be run
     at 400MHz.  If we want to get serious about ETU look-up performance, the SVK would need modification
     with a 100MHz crystal and 4X multiplier.
  */

  /* Lookups >= 320 cause IL flowcontrol.  This is not an error condition in and of itself. */
  if ( g_nDataBaseLayout == C3HPPC_ETU_LOOKUP__320 || g_nDataBaseLayout == C3HPPC_ETU_LOOKUP__640 ) {
    READ_ILAMAC_TX_INTR_STSr( pc3hppcTestInfo->nUnit, &uReg );
    if ( soc_reg_field_get( pc3hppcTestInfo->nUnit, ILAMAC_TX_INTR_STSr, uReg, TX_RDYOUTf ) ) {
      uReg = 0;
      soc_reg_field_set( pc3hppcTestInfo->nUnit, ILAMAC_TX_INTR_STSr, &uReg, TX_RDYOUTf, 1 );
      WRITE_ILAMAC_TX_INTR_CLEARr( pc3hppcTestInfo->nUnit, uReg );
      cli_out("\n\nWARNING:  *********  CLEARED ILAMAC_TX_INTR \"TX_RDYOUT\" flow control condition! *********\n\n" );
    }
  }
  /* Lookups == 640 cause NL11K flowcontrol.  This is not an error condition in and of itself. */
  if ( g_nDataBaseLayout == C3HPPC_ETU_LOOKUP__640 ) {
    READ_ILAMAC_RX_INTF_INTR0_STSr( pc3hppcTestInfo->nUnit, &uReg );
    if ( soc_reg_field_get( pc3hppcTestInfo->nUnit, ILAMAC_RX_INTF_INTR0_STSr, uReg, XOFF_TX_CH0f ) ||
         soc_reg_field_get( pc3hppcTestInfo->nUnit, ILAMAC_RX_INTF_INTR0_STSr, uReg, XOFF_TX_CH1f ) ) {
      uReg = 0;
      soc_reg_field_set( pc3hppcTestInfo->nUnit, ILAMAC_RX_INTF_INTR0_STSr, &uReg, XOFF_TX_CH0f, 1 );
      soc_reg_field_set( pc3hppcTestInfo->nUnit, ILAMAC_RX_INTF_INTR0_STSr, &uReg, XOFF_TX_CH1f, 1 );
      WRITE_ILAMAC_RX_INTF_INTR0_CLEARr( pc3hppcTestInfo->nUnit, uReg );
      cli_out("\n\nWARNING:  *********  CLEARED ILAMAC_RX_INTF_INTR0_STS \"XOFF_TX_CHx\" NL11K flow control condition! *********\n\n" );
    }
    READ_ETU_RX_RSP_FIFO_INTR_STSr( pc3hppcTestInfo->nUnit, &uReg );
    if ( soc_reg_field_get( pc3hppcTestInfo->nUnit, ETU_RX_RSP_FIFO_INTR_STSr, uReg, XOFF_RXf ) ) {
      uReg = 0;
      soc_reg_field_set( pc3hppcTestInfo->nUnit, ETU_RX_RSP_FIFO_INTR_STSr, &uReg, XOFF_RXf, 1 );
      WRITE_ETU_RX_RSP_FIFO_INTR_CLRr( pc3hppcTestInfo->nUnit, uReg );
      cli_out("\n\nWARNING:  *********  CLEARED ETU_RX_RSP_FIFO_INTR_STS \"XOFF_RX\" flow control condition! *********\n\n" );
    }
  }

 

  soc_cm_sfree(pc3hppcTestInfo->nUnit, g_pFlowTable);

  return 0;

#endif

}

#endif /* #ifdef BCM_CALADAN3_SUPPORT */
