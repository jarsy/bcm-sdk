/* $Id: c3hppc_ocm_store_test1.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/defs.h>


#ifdef BCM_CALADAN3_SUPPORT

#include "../c3hppc_test.h"
 

static c3hppc_64b_ocm_entry_template_t *g_pFlowTable;
static int g_nFlowTableSize;

static int g_nDmaContentionBlockSize;
static uint32 g_nDmaContentionBlockOffset;
static c3hppc_64b_ocm_entry_template_t *g_pDmaContentionOcmBlock;
static int g_nDmaContentionLrpPort;


int
c3hppc_ocm_store_test1__init(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  int rc, nCopInstance;

  c3hppc_populate_with_defaults( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );
  strcpy(pc3hppcTestInfo->BringUpControl.sUcodeFileName, "ocm_store.oasm");

  pc3hppcTestInfo->BringUpControl.uCmuBringUp = 0;
  rc = c3hppc_bringup( pc3hppcTestInfo->nUnit, &(pc3hppcTestInfo->BringUpControl) );
  if ( rc ) return 1;

  for ( nCopInstance = 0; nCopInstance < C3HPPC_COP_INSTANCE_NUM; ++nCopInstance ) {
    c3hppc_cop_segments_enable( pc3hppcTestInfo->nUnit, nCopInstance,
                                pc3hppcTestInfo->BringUpControl.aCopSegmentInfo[nCopInstance] );
  }

  return 0;
}

int
c3hppc_ocm_store_test1__run(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
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

  uSegmentTransferSize = (uint32) pc3hppcTestInfo->nTransferSize;
  uTransferSizeShift = C3HPPC_OCM_NUM_OF_TRANSFER_SIZES - uSegmentTransferSize - 1;

  cli_out("\nTransfer size --> %d bit(s)\n",  (1 << uSegmentTransferSize) );

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
   * 55:48 --> contains the shift(right) value that is used when updating the 64b entry shift accumulator, based on transfer size
   * 47:40 --> contains the 64b entry shift(right) accumulator
   * 39:32 --> contains the OCM offset shift(left) amount, based on transfer size 
   * 31:24 --> contains the number of iterations necessary to fill out a 64b entry based on transfer size
   * 23:16 --> contains the running count(decremented due to BigEndian) of iterations necessary to fill out a 64b entry
   * 15:8  --> contains the number of iterations necessary to fill up 9x16K OCM blocks
   *  7:0  --> contains the running count(incremented) of iterations necessary to fill up 9x16K OCM blocks
   *****************************************************************************************************************************/
  for ( nIndex = 0; nIndex < g_nFlowTableSize; ++nIndex ) {
    /* 144(9x16) iterations -- see comment below for explanation */
    g_pFlowTable[nIndex].uData[0] = 0x00009000 | ((1 << uTransferSizeShift) << 24) | ((1 << uTransferSizeShift) << 16);
    g_pFlowTable[nIndex].uData[1] = uTransferSizeShift | ((1 << uSegmentTransferSize) << 16);
  }
  nOcmPort = c3hppc_ocm_map_cop2ocm_port(0);
  c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, 0, 0, (g_nFlowTableSize-1), 1, g_pFlowTable[0].uData ); 

  nSegment = C3HPPC_OCM_NUM_OF_TRANSFER_SIZES;
  uSegmentBase = 1 * C3HPPC_OCM_MAX_PHY_BLK_SIZE_IN_ROWS;
  uSegmentLimit = ((9 * C3HPPC_OCM_MAX_PHY_BLK_SIZE_IN_ROWS) << uTransferSizeShift) - 1;
  for ( nLrpPort = 0; nLrpPort < C3HPPC_NUM_OF_OCM_LRP_PORTS; ++nLrpPort ) {
    if ( nLrpPort == 0 || nLrpPort == 6 ) {
      nStartingPhysicalBlock = ( nLrpPort == 0 ) ? 9 : 7;
    }
    nOcmPort = c3hppc_ocm_map_lrp2ocm_port(nLrpPort);
    c3hppc_ocm_port_program_segment_table(pc3hppcTestInfo->nUnit, nOcmPort, nSegment, uSegmentTransferSize,
                                          uSegmentBase, uSegmentLimit, 0);
    c3hppc_ocm_port_program_port_table(pc3hppcTestInfo->nUnit, nOcmPort, uSegmentBase,
                                       uSegmentLimit, uSegmentTransferSize, nStartingPhysicalBlock); 

    /* Setup a segment per port so DMA can be done on a 64b transfer size basis */
    c3hppc_ocm_port_program_segment_table(pc3hppcTestInfo->nUnit, nOcmPort, (nSegment+1), C3HPPC_DATUM_SIZE_QUADWORD,
                                          uSegmentBase, ((9 * C3HPPC_OCM_MAX_PHY_BLK_SIZE_IN_ROWS) - 1), 0);

    nStartingPhysicalBlock += 9;
  } 



  /*
   * In 1 epoch 128 FlowIDs execute.  Therefore it takes 8 epochs to cover a single round of 1024 FlowIDs.
   * To do 9x16K stores on each LRP port each FlowID is responsible for 9x16(144) iterations/rounds.
   * So this test must run a minimum of 1152(8x144) epochs to fill 9 OCM 16K blocks per port with
   * a transfer size of 64bits.  For 32b, 16b, 8b, ... there is a 2x, 4x, 8x ... multiplication factor  
   * as it takes multiple store operations to fill out the entire 64b entry. 
   */
  COMPILER_64_SET(nEpochCount, 0, 1152);
  COMPILER_64_SHL(nEpochCount, uTransferSizeShift);
  c3hppc_lrp_start_control( pc3hppcTestInfo->nUnit, nEpochCount );
  cli_out("\nIssued LRP \"start\" command!\n");

  /*
   * Setup DMA contention activity
   */
  g_nDmaContentionBlockSize = 1 * C3HPPC_OCM_MAX_PHY_BLK_SIZE_IN_ROWS;
  g_nDmaContentionBlockOffset = (sal_rand() % 9) * C3HPPC_OCM_MAX_PHY_BLK_SIZE_IN_ROWS;
  g_pDmaContentionOcmBlock = (c3hppc_64b_ocm_entry_template_t *)
                                 soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                               g_nDmaContentionBlockSize * sizeof(c3hppc_64b_ocm_entry_template_t),
                                               "dma_contention_ocm_block");
  nSegment = C3HPPC_OCM_NUM_OF_TRANSFER_SIZES + 1;
  g_nDmaContentionLrpPort = sal_rand() % C3HPPC_NUM_OF_OCM_LRP_PORTS;
  nOcmPort = c3hppc_ocm_map_lrp2ocm_port(g_nDmaContentionLrpPort);
  cli_out("\nLaunched DMA contention activity for LRP Port %d\n", g_nDmaContentionLrpPort);
  c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, nSegment, 
                             g_nDmaContentionBlockOffset, (g_nDmaContentionBlockOffset + g_nDmaContentionBlockSize-1),
                             0, g_pDmaContentionOcmBlock->uData ); 

  cli_out("\nDMA contention activity for LRP Port %d done!\n", g_nDmaContentionLrpPort);


  c3hppc_wait_for_test_done( pc3hppcTestInfo->nUnit );
  cli_out("\nDetected LRP \"offline\" event\n");



/*
  uint32 auOcmEntry[6];

  nSegment = C3HPPC_DATUM_SIZE_QUADWORD;
  nOcmPort = c3hppc_ocm_map_lrp2ocm_port(0);

  auOcmEntry[0] = 0x12345678;
  auOcmEntry[1] = 0x87654321;
  c3hppc_ocm_mem_read_write( pc3hppcTestInfo->nUnit, nOcmPort, nSegment, 0x1fffd, 1, auOcmEntry ); 

  auOcmEntry[0] = auOcmEntry[1] = 0;
  c3hppc_ocm_mem_read_write( pc3hppcTestInfo->nUnit, nOcmPort, nSegment, 0x1fffd, 0, auOcmEntry );
  cli_out("auOcmEntry --> 0x%08x 0x%08x \n", auOcmEntry[1], auOcmEntry[0]);

*/



  return 0;
}

int
c3hppc_ocm_store_test1__done(c3hppc_test_info_t *pc3hppcTestInfo, void *pUserData)
{
  c3hppc_64b_ocm_entry_template_t *pOcmBlock;
  int nDmaBlockSize;
  c3hppc_64b_ocm_entry_template_t uOcmEntryExpect;
  c3hppc_64b_ocm_entry_template_t uXORa, uXORb;
  int nErrorCnt, nDmaContentionErrorCnt;
  int nIndex, nDmaContentionIndex;
  int nOcmPort;
  int nLrpPort;
  int nSegment;


  /* Check FlowTable contents at end of test */
  uOcmEntryExpect.uData[1] = g_pFlowTable[0].uData[1];
  uOcmEntryExpect.uData[0] = g_pFlowTable[0].uData[0] | ((g_pFlowTable[0].uData[0] >> 8) & 0xff);

  nErrorCnt = 0;
  nOcmPort = c3hppc_ocm_map_cop2ocm_port(0);
  c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, 0, 0, (g_nFlowTableSize-1), 0, g_pFlowTable[0].uData ); 
  for ( nIndex = 0; nIndex < g_nFlowTableSize; ++nIndex ) {
    if ( g_pFlowTable[nIndex].uData[1] != uOcmEntryExpect.uData[1] ||
         g_pFlowTable[nIndex].uData[0] != uOcmEntryExpect.uData[0] ) {
      if ( nErrorCnt < 10 ) {
        test_error( pc3hppcTestInfo->nUnit,
                    "FlowTable entry[%d] --> Actual: 0x%08x_%08x   Expect: 0x%08x_%08x\n",
                    nIndex, g_pFlowTable[nIndex].uData[1], g_pFlowTable[nIndex].uData[0],
                    uOcmEntryExpect.uData[1], uOcmEntryExpect.uData[0] );
      }
      ++nErrorCnt;
      pc3hppcTestInfo->nTestStatus = TEST_FAIL;
    }
  }

  /* Check OCM contents at end of test */
  nDmaBlockSize = 9 * C3HPPC_OCM_MAX_PHY_BLK_SIZE_IN_ROWS;
  pOcmBlock = (c3hppc_64b_ocm_entry_template_t *) soc_cm_salloc(pc3hppcTestInfo->nUnit,
                                                                nDmaBlockSize * sizeof(c3hppc_64b_ocm_entry_template_t),
                                                                "ocm_block");
  nSegment = C3HPPC_OCM_NUM_OF_TRANSFER_SIZES + 1;
  for ( nLrpPort = 0; nLrpPort < C3HPPC_NUM_OF_OCM_LRP_PORTS; ++nLrpPort ) {
    cli_out("\nChecking OCM contents for LRP Port %d\n", nLrpPort);
    nOcmPort = c3hppc_ocm_map_lrp2ocm_port(nLrpPort);
    c3hppc_ocm_dma_read_write( pc3hppcTestInfo->nUnit, nOcmPort, nSegment, 
                               0, (nDmaBlockSize-1), 0, pOcmBlock->uData ); 
    nErrorCnt = 0;
    nDmaContentionErrorCnt = 0;
    nDmaContentionIndex = 0;
    uOcmEntryExpect.uData[0] = (nLrpPort << 16) | (nLrpPort << 20) | (nLrpPort << 24) | (nLrpPort << 28);
    for ( nIndex = 0; nIndex < nDmaBlockSize; ++nIndex ) {
      uOcmEntryExpect.uData[1] = 0xfedcb000 | (nIndex & 0x3ff);
      uOcmEntryExpect.uData[0] &= 0xfffffc00;
      uOcmEntryExpect.uData[0] |= nIndex >> 10;
      if ( pOcmBlock[nIndex].uData[1] != uOcmEntryExpect.uData[1] ||
           pOcmBlock[nIndex].uData[0] != uOcmEntryExpect.uData[0] ) {
        if ( nErrorCnt < 16 ) {
          test_error( pc3hppcTestInfo->nUnit,
                      "LrpPort%d OCM entry[%d] --> Actual: 0x%08x_%08x   Expect: 0x%08x_%08x\n",
                      nLrpPort, nIndex, pOcmBlock[nIndex].uData[1], pOcmBlock[nIndex].uData[0],
                      uOcmEntryExpect.uData[1], uOcmEntryExpect.uData[0] );
        }
        ++nErrorCnt;
        pc3hppcTestInfo->nTestStatus = TEST_FAIL;
      }

      if ( g_nDmaContentionLrpPort == nLrpPort && nIndex >= g_nDmaContentionBlockOffset &&
           nIndex < (g_nDmaContentionBlockOffset + g_nDmaContentionBlockSize) ) {
        /*
         * For non 64b transfer sizes it is possible for the DMA to read out an incomplete 64b entry
         * in OCM.  The following double "XOR" will handle this condition.
         */
        uXORa.uData[1] = g_pDmaContentionOcmBlock[nDmaContentionIndex].uData[1] ^ uOcmEntryExpect.uData[1];
        uXORa.uData[0] = g_pDmaContentionOcmBlock[nDmaContentionIndex].uData[0] ^ uOcmEntryExpect.uData[0];
        uXORb.uData[1] = uXORa.uData[1] ^ uOcmEntryExpect.uData[1];
        uXORb.uData[0] = uXORa.uData[0] ^ uOcmEntryExpect.uData[0];
        if ( (g_pDmaContentionOcmBlock[nDmaContentionIndex].uData[1] != uXORb.uData[1] &&
              g_pDmaContentionOcmBlock[nDmaContentionIndex].uData[1] != 0) ||
             (g_pDmaContentionOcmBlock[nDmaContentionIndex].uData[0] != uXORb.uData[0] &&
              g_pDmaContentionOcmBlock[nDmaContentionIndex].uData[0] != 0) ) {
          if ( nDmaContentionErrorCnt < 64 ) {
            test_error( pc3hppcTestInfo->nUnit,
                        "LrpPort%d DMA Contention OCM entry[%d] --> Actual: 0x%08x_%08x   Expect: 0x%08x_%08x\n",
                        nLrpPort, nIndex, g_pDmaContentionOcmBlock[nDmaContentionIndex].uData[1],
                        g_pDmaContentionOcmBlock[nDmaContentionIndex].uData[0],
                        uOcmEntryExpect.uData[1], uOcmEntryExpect.uData[0] );
          }
          ++nDmaContentionErrorCnt;
          pc3hppcTestInfo->nTestStatus = TEST_FAIL;
        }
        ++nDmaContentionIndex;
      }

    }
  } 


  soc_cm_sfree(pc3hppcTestInfo->nUnit, g_pFlowTable);
  soc_cm_sfree(pc3hppcTestInfo->nUnit, pOcmBlock);
  soc_cm_sfree(pc3hppcTestInfo->nUnit, g_pDmaContentionOcmBlock);

  return 0;
}

#endif /* #ifdef BCM_CALADAN3_SUPPORT */
