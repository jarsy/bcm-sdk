/* $Id: c3hppc_ocm_load_test1.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/defs.h>


#ifdef BCM_CALADAN3_SUPPORT

#include "../c3hppc_test.h"
 

static c3hppc_64b_ocm_entry_template_t *g_pFlowTable;
static int g_nFlowTableSize;


static int    g_nDmaContentionLrpPort;
static uint32 g_nDmaContentionBlockOffset;

int
c3hppc_ocm_load_test1__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  int rc, nCopInstance;

  c3hppc_populate_with_defaults( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );
  strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "ocm_load_xfer64b.oasm");

  pc3hppcTestInfo->BringUpControl.uCmuBringUp = 0;
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
c3hppc_ocm_load_test1__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{

  uint64 nEpochCount;
  int nOcmPort;
  int nLrpPort;
  int nSegment;
  int nIndex;
  uint32 uSegmentBase, uSegmentLimit;
  int nStartingPhysicalBlock = 0;
  uint32 uSegmentTransferSize;
  uint32 uTransferSizeShift;
  c3hppc_64b_ocm_entry_template_t *pOcmBlock;
  int nDmaBlockSize;
  c3hppc_64b_ocm_entry_template_t uOcmEntryInit;
  c3hppc_64b_ocm_entry_template_t *pDmaContentionWriteBlock, *pDmaContentionReadBlock;

  uSegmentTransferSize = (uint32) pc3hppcTestInfo->nTransferSize;
  assert(uSegmentTransferSize == C3HPPC_DATUM_SIZE_QUADWORD);
  uTransferSizeShift = C3HPPC_OCM_NUM_OF_TRANSFER_SIZES - uSegmentTransferSize - 1;

  cli_out("\nTransfer size --> %d bit(s)\n",  (1 << uSegmentTransferSize) );

  pDmaContentionWriteBlock = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                                                               sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                               "write buffer");
  pDmaContentionReadBlock = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                                                              sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                              "read buffer");

  g_nFlowTableSize = 1024;
  g_pFlowTable = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                                                   g_nFlowTableSize * sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                   "flow_table");
  sal_memset( g_pFlowTable, 0, (g_nFlowTableSize * sizeof(c3hppc_64b_ocm_entry_template_t)) );
  /****************************************************************************************************************************
   * A FlowTable entry contains the parameters to support 2 loop controls the outer loop being the number
   * of OCM 64b locations to write and the inner loop being the number of operations to fill a 64b entry
   * based on the transfer size.
   *               
   * 63:32 --> contains the actual miscompare data
   * 31:31 --> contains the error flag
   * 27:24 --> contains the first port that experienced a miscompare
   * 15:8  --> contains the number of iterations necessary to fill up 9x16K OCM blocks
   *  7:0  --> contains the running count(incremented) of iterations necessary to fill up 9x16K OCM blocks
   *****************************************************************************************************************************/
  for ( nIndex = 0; nIndex < g_nFlowTableSize; ++nIndex ) {
    /* 144(9x16) iterations -- see comment below for explanation */
    g_pFlowTable[nIndex].uData[0] = 0x00009000;
  }
  nOcmPort = c3hppc_ocm_map_cop2ocm_port(0);
  c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, 0, 0, (g_nFlowTableSize-1), 1, g_pFlowTable[0].uData ); 

  nSegment = C3HPPC_OCM_NUM_OF_TRANSFER_SIZES;
  /****************************************************************************************************************************
   * "uSegmentBase" will be setup (assuming "uDefaultPhysicalSegmentSizeIn16kBlocks" == 1) such that 
   * OC_LRP_PORTX_BLOCK table entries 1 through 9 will resolve the physical address.  Each ports physical address space
   * will span 9 successive 16K blocks.  Port0 will start at physical block 9 (Port1 --> 18, Port2 --> 27, ...)
   * because 1 16K block is reserved for each port for general testcase operation.  9 ports are attached to OCM bank 0.
   ***************************************************************************************************************************/
  uSegmentBase = pc3hppcTestInfo->BringUpControl.uDefaultPhysicalSegmentSizeIn16kBlocks * C3HPPC_OCM_MAX_PHY_BLK_SIZE_IN_ROWS;
  uSegmentLimit = ((9 * C3HPPC_OCM_MAX_PHY_BLK_SIZE_IN_ROWS) << uTransferSizeShift) - 1;
  for ( nLrpPort = 0; nLrpPort < C3HPPC_NUM_OF_OCM_LRP_PORTS; ++nLrpPort ) {
    if ( nLrpPort == 0 || nLrpPort == 6 ) {
      nStartingPhysicalBlock = ( nLrpPort == 0 ) ? c3hppc_test__get_ocm_next_physical_block_0to63() : 
                                                   c3hppc_test__get_ocm_next_physical_block_64to127();
    }
    nOcmPort = c3hppc_ocm_map_lrp2ocm_port(nLrpPort);
    c3hppc_ocm_port_program_segment_table(pc3hppcTestInfo->nUnit, nOcmPort, nSegment, uSegmentTransferSize,
                                          uSegmentBase, uSegmentLimit, 1);
    c3hppc_ocm_port_program_port_table(pc3hppcTestInfo->nUnit, nOcmPort, uSegmentBase,
                                       uSegmentLimit, uSegmentTransferSize, nStartingPhysicalBlock); 

    /* Setup a segment per port so DMA can be done on a 64b transfer size basis */
    c3hppc_ocm_port_program_segment_table(pc3hppcTestInfo->nUnit, nOcmPort, (nSegment+1), C3HPPC_DATUM_SIZE_QUADWORD,
                                          uSegmentBase, ((9 * C3HPPC_OCM_MAX_PHY_BLK_SIZE_IN_ROWS) - 1), 1);

    nStartingPhysicalBlock += 9;
  } 

  /****************************************************************************************************************************
   * Initialize OCM contents
   ***************************************************************************************************************************/
  nDmaBlockSize = 9 * C3HPPC_OCM_MAX_PHY_BLK_SIZE_IN_ROWS;
  pOcmBlock = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                                                nDmaBlockSize * sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                "ocm_block");
  nSegment = C3HPPC_OCM_NUM_OF_TRANSFER_SIZES + 1;
  for ( nLrpPort = 0; nLrpPort < C3HPPC_NUM_OF_OCM_LRP_PORTS; ++nLrpPort ) {
    uOcmEntryInit.uData[0] = nLrpPort << 12;
    uOcmEntryInit.uData[1] = nLrpPort << 16;
    for ( nIndex = 0; nIndex < nDmaBlockSize; ++nIndex ) {
      uOcmEntryInit.uData[0] &= 0x0000f000;
      uOcmEntryInit.uData[0] |= nIndex & 0x3ff;
      uOcmEntryInit.uData[1] &= 0x000f0000;
      uOcmEntryInit.uData[1] |= (nIndex >> 10) << 20;
      pOcmBlock[nIndex].uData[1] = uOcmEntryInit.uData[1];
      pOcmBlock[nIndex].uData[0] = uOcmEntryInit.uData[0];
    }
    cli_out("\nInitializing OCM contents for LRP Port %d\n", nLrpPort);
    nOcmPort = c3hppc_ocm_map_lrp2ocm_port(nLrpPort);
    c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, nSegment, 
                               0, (nDmaBlockSize-1), 1, pOcmBlock->uData ); 
  }
  soc_cm_sfree(pc3hppcTestInfo->nUnit, pOcmBlock);


/* Method used to cause "rerror" assertion from oc to lr
  nSegment = C3HPPC_OCM_NUM_OF_TRANSFER_SIZES;
  for ( nLrpPort = 0; nLrpPort < C3HPPC_NUM_OF_OCM_LRP_PORTS; ++nLrpPort ) {
    nOcmPort = c3hppc_ocm_map_lrp2ocm_port(nLrpPort);
    c3hppc_ocm_port_modify_segment_error_protection(pc3hppcTestInfo->nUnit, nOcmPort, nSegment, C3HPPC_ERROR_PROTECTION__PARITY);
  } 
*/


  /*
   * In 1 epoch 128 FlowIDs execute.  Therefore it takes 8 epochs to cover a single round of 1024 FlowIDs.
   * To do 9x16K loads on each LRP port each FlowID is responsible for 9x16(144) iterations/rounds.
   * A burst of 8 consecutive (no gaps) load operations are done per epoch meaning 8 iterations are covered
   * per epoch.  The end result is this test must run a minimum of {[1152(8x144)] / 8} 144 epochs to
   * fetch 9 OCM 16K blocks per port with a transfer size of 64bits.
   */

  COMPILER_64_SET(nEpochCount, 0, 1152);
  COMPILER_64_SHL(nEpochCount, uTransferSizeShift);
  COMPILER_64_SHR(nEpochCount, 3);
  c3hppc_lrp_start_control( pc3hppcTestInfo->nUnit, nEpochCount );
  cli_out("\nIssued LRP \"start\" command!\n");

  /*
   * Setup DMA contention activity
   */
  g_nDmaContentionBlockOffset = (sal_rand() % 9) * C3HPPC_OCM_MAX_PHY_BLK_SIZE_IN_ROWS;
  pDmaContentionWriteBlock->uData[0] = pDmaContentionWriteBlock->uData[1] = 0x12345678;
  nSegment = C3HPPC_OCM_NUM_OF_TRANSFER_SIZES + 1;
  g_nDmaContentionLrpPort = sal_rand() % C3HPPC_NUM_OF_OCM_LRP_PORTS;
  nOcmPort = c3hppc_ocm_map_lrp2ocm_port(g_nDmaContentionLrpPort);
  cli_out("\nLaunched DMA contention activity for LRP Port %d\n", g_nDmaContentionLrpPort);
  c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, nSegment, 
                             g_nDmaContentionBlockOffset, g_nDmaContentionBlockOffset,
                             1, pDmaContentionWriteBlock->uData );

  c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, nSegment, 
                             g_nDmaContentionBlockOffset, g_nDmaContentionBlockOffset,
                             0, pDmaContentionReadBlock->uData );

  if ( pDmaContentionWriteBlock->uData[1] != pDmaContentionReadBlock->uData[1] ||
       pDmaContentionWriteBlock->uData[0] != pDmaContentionReadBlock->uData[0] ) {
    test_error( pc3hppcTestInfo->nUnit,
                "DMA entry[%d] --> Actual: 0x%08x_%08x   Expect: 0x%08x_%08x\n",
                g_nDmaContentionBlockOffset, pDmaContentionReadBlock->uData[1], pDmaContentionReadBlock->uData[0],
                pDmaContentionWriteBlock->uData[1], pDmaContentionWriteBlock->uData[0] );
    pc3hppcTestInfo->nTestStatus = TEST_FAIL;
  }

  c3hppc_wait_for_test_done( pc3hppcTestInfo->nUnit );
  cli_out("\nDetected LRP \"offline\" event\n");

  soc_cm_sfree(pc3hppcTestInfo->nUnit, pDmaContentionWriteBlock);
  soc_cm_sfree(pc3hppcTestInfo->nUnit, pDmaContentionReadBlock);

  return 0;
}

int
c3hppc_ocm_load_test1__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  c3hppc_64b_ocm_entry_template_t uOcmEntryExpect;
  int nErrorCnt;
  int nIndex;
  int nOcmPort;


  /* Check FlowTable contents at end of test */
  uOcmEntryExpect.uData[1] = g_pFlowTable[0].uData[1];
  uOcmEntryExpect.uData[0] = g_pFlowTable[0].uData[0] | ((g_pFlowTable[0].uData[0] >> 8) & 0xff);

  nErrorCnt = 0;
  nOcmPort = c3hppc_ocm_map_cop2ocm_port(0);
  c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, 0, 0, (g_nFlowTableSize-1), 0, g_pFlowTable[0].uData ); 
  for ( nIndex = 0; nIndex < g_nFlowTableSize; ++nIndex ) {
    if ( g_pFlowTable[nIndex].uData[1] != uOcmEntryExpect.uData[1] ||
         g_pFlowTable[nIndex].uData[0] != uOcmEntryExpect.uData[0] ) {
      if ( g_pFlowTable[nIndex].uData[1] == 0x12345678 &&
           ((g_pFlowTable[nIndex].uData[0] >> 24) == (g_nDmaContentionLrpPort | 0x80)) &&
           nIndex == 0 ) { 
        cli_out("\nDMA data detected correctly!!\n");
      } else if ( nErrorCnt < 10 ) {
        test_error( pc3hppcTestInfo->nUnit,
                    "FlowTable entry[%d] --> Actual: 0x%08x_%08x   Expect: 0x%08x_%08x\n",
                    nIndex, g_pFlowTable[nIndex].uData[1], g_pFlowTable[nIndex].uData[0],
                    uOcmEntryExpect.uData[1], uOcmEntryExpect.uData[0] );
        ++nErrorCnt;
        pc3hppcTestInfo->nTestStatus = TEST_FAIL;
      }
    }
  }

  soc_cm_sfree(pc3hppcTestInfo->nUnit, g_pFlowTable);

  return 0;
}

#endif /* #ifdef BCM_CALADAN3_SUPPORT */
