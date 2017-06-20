/* $Id: c3hppc_rce_test2.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/defs.h>


#ifdef BCM_CALADAN3_SUPPORT

#include "../c3hppc_test.h"
 
#define C3HPPC_RCE_TEST2__STREAM_NUM                (1)
#define C3HPPC_RCE_TEST2__MAX_KEYS                  (0x08000)
#define C3HPPC_RCE_TEST2__RESULT_PORT               (9)
#define C3HPPC_RCE_TEST2__MAX_PROGRAM_NUM           (4)

static c3hppc_64b_ocm_entry_template_t *g_pFlowTable;
static int g_nFlowTableSize;

static uint32 g_auLpmProgramFilterSetNumber[C3HPPC_RCE_TEST2__MAX_PROGRAM_NUM];
static uint32 g_auLpmProgramFilterSetLength[C3HPPC_RCE_TEST2__MAX_PROGRAM_NUM];
static uint32 g_auLpmProgramKeyLength[C3HPPC_RCE_TEST2__MAX_PROGRAM_NUM];
static uint32 g_auLpmProgramKeyStartIndex[C3HPPC_RCE_TEST2__MAX_PROGRAM_NUM];
static int g_nNumberOfPrograms;


int
c3hppc_rce_test2__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  int rc, nCopInstance;
  
  pc3hppcTestInfo->BringUpControl.uDefaultPhysicalSegmentSizeIn16kBlocks = 4;
  c3hppc_populate_with_defaults( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );
  pc3hppcTestInfo->BringUpControl.uRceBringUp = 1;
  pc3hppcTestInfo->BringUpControl.bLrpMaximizeActiveContexts = 0;

  g_nNumberOfPrograms = 1; 

  strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "rce_test2.oasm");
  COMPILER_64_SET(pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition,0,1);
  /* 69 for "switch 1" and 129 for "switch 2" */
  g_auLpmProgramFilterSetNumber[0] = C3HPPC_MAX(129,pc3hppcTestInfo->nSetupOptions);
  g_auLpmProgramFilterSetLength[0] = 32;
  g_auLpmProgramKeyLength[0] = 16;
  g_auLpmProgramKeyStartIndex[0] = 0;


  

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
c3hppc_rce_test2__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{

  int nOcmPort;
  uint32 uReg, uContexts, uFlowID, uKey;
  uint64 auuKeyData[7], uuTmp;
  uint32 uFilterSetIndex, uSBlkIndex, uColumnIndex;
  int nProgramNumber;
  uint32 uProgramBaseAddress;
  int nDoSkips, nDoNoMatches;
  int nLpmProgram;
  c3hppc_64b_ocm_entry_template_t *pOcmBlock;
  int nDmaBlockSize;

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
  nDoSkips = 0;
  nDoNoMatches = 0;

  for ( uFlowID = 0; uFlowID < g_nFlowTableSize; ++uFlowID ) {
    g_pFlowTable[uFlowID].uData[0] = nDoSkips | nDoNoMatches;
  }
  nOcmPort = c3hppc_ocm_map_cop2ocm_port(0);
  c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, 0, 0, (g_nFlowTableSize-1), 1, g_pFlowTable[0].uData );

  /* The following allocation is used to initialize OCM memory with RCE results.  */
  nDmaBlockSize = C3HPPC_RCE_TEST2__MAX_KEYS;
  pOcmBlock = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                                                nDmaBlockSize * sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                "ocm_block");
  nOcmPort = c3hppc_ocm_map_lrp2ocm_port(C3HPPC_RCE_TEST2__RESULT_PORT);

  /****************************************************************************************************************************
   * Build RCE program and filter pattern image.
   *****************************************************************************************************************************/
  for ( nLpmProgram = 0; nLpmProgram < g_nNumberOfPrograms; ++nLpmProgram ) {

    nProgramNumber = 10 + nLpmProgram;
    uProgramBaseAddress = (C3HPPC_RCE_INSTRUCTION_NUM / C3HPPC_RCE_TEST2__MAX_PROGRAM_NUM) * nLpmProgram;

    c3hppc_rce_create_program_for_lpm_exact_match( pc3hppcTestInfo->nUnit, nProgramNumber, uProgramBaseAddress,
                                                   g_auLpmProgramFilterSetNumber[nLpmProgram],
                                                   g_auLpmProgramFilterSetLength[nLpmProgram],
                                                   g_auLpmProgramKeyLength[nLpmProgram],
                                                   g_auLpmProgramKeyStartIndex[nLpmProgram], 0 );

    c3hppc_lrp_setup_rce_program( pc3hppcTestInfo->nUnit, nProgramNumber, (uint32) nProgramNumber );

    COMPILER_64_ZERO(auuKeyData[0]);

    for ( uKey = 0; uKey < (g_auLpmProgramFilterSetNumber[nLpmProgram] * C3HPPC_RCE_TOTAL_COLUMN_NUM); ++uKey ) {

      COMPILER_64_SET(auuKeyData[0],0,uKey);

      uFilterSetIndex = uKey / C3HPPC_RCE_TOTAL_COLUMN_NUM;
      uSBlkIndex = (uKey % C3HPPC_RCE_TOTAL_COLUMN_NUM) / C3HPPC_RCE_NUM_COLUMNS_PER_SBLOCK;
      uColumnIndex = (uKey % C3HPPC_RCE_TOTAL_COLUMN_NUM) & (C3HPPC_RCE_NUM_COLUMNS_PER_SBLOCK - 1);

      c3hppc_rce_add_filter_for_lpm_exact_match( g_auLpmProgramFilterSetLength[nLpmProgram], g_auLpmProgramKeyLength[nLpmProgram],
                                                 uFilterSetIndex, uSBlkIndex, uColumnIndex, uProgramBaseAddress, auuKeyData );

      if ( uKey < nDmaBlockSize ) {
        pOcmBlock[uKey].uData[0] = (uFilterSetIndex << 12) | (uSBlkIndex << 5) | uColumnIndex;
        pOcmBlock[uKey].uData[1] = 0; 
      }

    } /* for ( uKey = 0; uKey < C3HPPC_RCE_TEST2__MAX_KEYS; ++uKey ) { */

    c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, C3HPPC_DATUM_SIZE_QUADWORD, 
                               0, (nDmaBlockSize-1), 1, pOcmBlock->uData );

  }  /* for ( nLpmProgram = 0; nLpmProgram < g_nNumberOfPrograms; ++nLpmProgram ) { */

  soc_cm_sfree(pc3hppcTestInfo->nUnit,pOcmBlock);



  cli_out("\nLoading the RCE image ...\n");
  c3hppc_rce_dma_image( pc3hppcTestInfo->nUnit );



  READ_LRA_CONFIG1r( pc3hppcTestInfo->nUnit, &uReg );
  uContexts = soc_reg_field_get( pc3hppcTestInfo->nUnit, LRA_CONFIG1r, uReg, CONTEXTSf );
  COMPILER_64_SET(pc3hppcTestInfo->uuEpochCount, COMPILER_64_HI(pc3hppcTestInfo->uuIterations), COMPILER_64_LO(pc3hppcTestInfo->uuIterations));
  COMPILER_64_UMUL_32(pc3hppcTestInfo->uuEpochCount, C3HPPC_RCE_TEST2__STREAM_NUM * uContexts);
  COMPILER_64_SET(uuTmp, COMPILER_64_HI(pc3hppcTestInfo->uuEpochCount), COMPILER_64_LO(pc3hppcTestInfo->uuEpochCount));
  COMPILER_64_ADD_64(uuTmp, pc3hppcTestInfo->uuExtraEpochsDueToSwitchStartupCondition);
  c3hppc_lrp_start_control( pc3hppcTestInfo->nUnit, uuTmp);


  while ( !c3hppc_is_test_done( pc3hppcTestInfo->nUnit ) ) {
    sal_sleep(1);
  }


  return 0;
}

int
c3hppc_rce_test2__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  int nErrorCnt;
  uint32 uFlowID;
  c3hppc_64b_ocm_entry_template_t uOcmEntryExpect;
  int nOcmPort;


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

  if ( nErrorCnt ) {
    pc3hppcTestInfo->nTestStatus = TEST_FAIL;
    cli_out("\n<c3hppc_rce_test2__done> -- Total error count --> %d\n", nErrorCnt );
  }

  soc_cm_sfree(pc3hppcTestInfo->nUnit, g_pFlowTable);

  return 0;
}

#endif /* #ifdef BCM_CALADAN3_SUPPORT */
