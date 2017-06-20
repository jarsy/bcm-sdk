/*
 * $Id: c3hppc_rce.c,v 1.9.30.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    c3hppc_rce.c
 * Purpose: Caladan3 RCE test driver
 * Requires:
 */


#include <shared/bsl.h>

#include <soc/defs.h>
#include <soc/mem.h>


#ifdef BCM_CALADAN3_SUPPORT

#include <appl/test/caladan3/c3hppc_rce.h>


static int g_anErrorRegisters[] = { RC_GLOBAL_ERRORr,
                                    RC_ECC_ERRORr,
                                    RC_ECC_STATUS0r,
                                    RC_ECC_STATUS1r
                                  };
static int g_nErrorRegistersCount = COUNTOF(g_anErrorRegisters);

static int g_anErrorMaskRegisters[] = { RC_GLOBAL_ERROR_MASKr,
                                        RC_ECC_ERROR_MASKr
                                      };
static int g_nErrorMaskRegistersCount = COUNTOF(g_anErrorMaskRegisters);

static uint64                           g_RceInstructionArray[C3HPPC_RCE_INSTRUCTION_NUM];
static c3hppc_rce_sblk_pattern_column_t g_RcePatternArray[C3HPPC_RCE_SBLOCK_NUM][C3HPPC_RCE_NUM_COLUMNS_PER_SBLOCK];


static uint64 g_auuResultsRuleCounters[C3HPPC_RCE_RESULTS_COUNTER_NUM];

int c3hppc_rce_hw_init( int nUnit, c3hppc_rce_control_info_t *pC3RceControlInfo ) {

  uint32 uRegisterValue;
  sal_time_t TimeStamp;

  sal_memset( g_RcePatternArray, 0x00, sizeof(g_RcePatternArray) );
  sal_memset( g_auuResultsRuleCounters, 0x00, sizeof(g_auuResultsRuleCounters) );


  /*
   * Take block out of soft reset
  */
  READ_RC_GLOBAL_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, RC_GLOBAL_CONFIGr, &uRegisterValue, SOFT_RESET_Nf, 1 );
  WRITE_RC_GLOBAL_CONFIGr( nUnit, uRegisterValue );

  READ_RC_GLOBAL_DEBUGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, RC_GLOBAL_DEBUGr, &uRegisterValue, SBLK_CLK_GATING_DISABLEf, 1 );
  soc_reg_field_set( nUnit, RC_GLOBAL_DEBUGr, &uRegisterValue, MEM_INITf, 1 );
  WRITE_RC_GLOBAL_DEBUGr( nUnit, uRegisterValue );

  if ( c3hppcUtils_poll_field( nUnit, REG_PORT_ANY, RC_GLOBAL_DEBUGr, MEM_INIT_DONEf, 1, 100, 1, &TimeStamp ) ) {
    cli_out("<c3hppc_rce_hw_init> -- RC_GLOBAL_DEBUG \"MEM_INIT_DONE\" event TIMEOUT!!!\n");
    return -1;
  }

  return 0;
}



int c3hppc_rce_mem_read_write( int nUnit, uint32 uOffset, 
                               uint8 bWrite, uint32 *puEntryData )
{
  uint32 u32bRegisterValue = 0;

  /* treat as logical access */
  soc_reg_field_set(nUnit, OC_CONFIGr, &u32bRegisterValue, SOFT_RESET_Nf, 1);
  WRITE_OC_CONFIGr(nUnit, u32bRegisterValue);

  if ( bWrite ) {
    SOC_IF_ERROR_RETURN(WRITE_OC_MEMORYm(nUnit, MEM_BLOCK_ANY, uOffset, puEntryData));
  } else {
    SOC_IF_ERROR_RETURN(READ_OC_MEMORYm(nUnit, MEM_BLOCK_ANY, uOffset, puEntryData));
  }

  return 0;
}


int c3hppc_rce_dma_read_write( int nUnit, uint32 uStartOffset, uint32 uEndOffset,
                               uint8 bWrite, uint32 *puDmaData )
{
  int rc;
  uint32 u32bRegisterValue = 0;

  /* treat as logical access */
  soc_reg_field_set(nUnit, OC_CONFIGr, &u32bRegisterValue, SOFT_RESET_Nf, 1);
  WRITE_OC_CONFIGr(nUnit, u32bRegisterValue);

  if ( bWrite ) {
    /*    coverity[negative_returns : FALSE]    */
    rc = soc_mem_write_range(nUnit, OC_MEMORYm, MEM_BLOCK_ANY, (int) uStartOffset,
                             (int) uEndOffset, (void *) puDmaData);
    if ( rc ) {
        cli_out("<c3hppc_rce_dma_read_write>  soc_mem_write_range rc --> %d\n", rc);
    }
  } else {
    rc = soc_mem_read_range(nUnit, OC_MEMORYm, MEM_BLOCK_ANY, (int) uStartOffset,
                            (int) uEndOffset, (void *) puDmaData);
    if ( rc ) {
        cli_out("<c3hppc_rce_dma_read_write>  soc_mem_read_range rc --> %d\n", rc);
    }
  }

  return rc;
}





int c3hppc_rce_hw_cleanup( int nUnit ) {

  int nIndex;

  for ( nIndex = 0; nIndex < g_nErrorRegistersCount; ++nIndex ) {
    soc_reg32_set( nUnit, g_anErrorRegisters[nIndex], SOC_BLOCK_PORT(nUnit,0), 0,
                              ((nIndex < g_nErrorMaskRegistersCount) ? 0xffffffff : 0x00000000) );
  }
  for ( nIndex = 0; nIndex < g_nErrorMaskRegistersCount; ++nIndex ) {
    soc_reg32_set( nUnit, g_anErrorMaskRegisters[nIndex], SOC_BLOCK_PORT(nUnit,0), 0, 0x00000000 );
  }

  return 0;
}



int c3hppc_rce_display_error_state( int nUnit ) {

  int rc, nIndex;

  for ( rc = 0, nIndex = 0; nIndex < g_nErrorRegistersCount; ++nIndex ) {
    rc += c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,0), g_anErrorRegisters[nIndex] );
  }

  return rc;
}




int c3hppc_rce_create_program_for_lpm_exact_match( int nUnit, int nProgramNumber, uint32 uProgramBaseAddress,
                                                   uint32 uNumberOfFilterSets, uint32 uFilterLength,
                                                   uint32 uKeyLength, uint32 uKeyStartIndex,
                                                   int nResultRegsSelect )
{
  uint32 uInstructionIndex, uFilterSetIndex, uLocalFilterLength, uMaxLength; 
  int    nIndex, nNopCount;
  c3hppc_rce_lpm_instruction_ut    LpmInstructionTemplate;
  c3hppc_rce_nop_instruction_ut    NopInstructionTemplate;
  c3hppc_rce_prefix_instruction_ut PrefixInstructionTemplate;
  c3hppc_rce_start_instruction_ut  StartInstructionTemplate;
  uint32 uRegisterValue, uKeyBitIndex, uLink;

  COMPILER_64_ZERO(NopInstructionTemplate.value);
  COMPILER_64_ZERO(LpmInstructionTemplate.value);
  LpmInstructionTemplate.bits.Opcode = 1;
  LpmInstructionTemplate.bits.POp = 1;
  LpmInstructionTemplate.bits.LUT = 0xff00;
  COMPILER_64_ZERO(PrefixInstructionTemplate.value);
  PrefixInstructionTemplate.bits.Opcode = 2;
  COMPILER_64_ZERO(StartInstructionTemplate.value);
  uKeyBitIndex = 0;
  uLink = 0;

  uMaxLength = 0;
  for ( uFilterSetIndex = 0, uInstructionIndex = uProgramBaseAddress; uFilterSetIndex < uNumberOfFilterSets; uFilterSetIndex++ ) {
    /* RCE BUG:  Jira -- CA3-2684 */
    uLocalFilterLength = ( uFilterSetIndex == 0 ) ? C3HPPC_MAX(uFilterLength, 40) : uFilterLength;
    uMaxLength += uLocalFilterLength; 

    nNopCount = (int) (uLocalFilterLength - uKeyLength - 2);  /* -2 comes from the valid rule/filter set and delimeter bits */
    StartInstructionTemplate.bits.Opcode = ( uFilterSetIndex == 0 ) ? 7 : 6; 
    uLink = ( uFilterSetIndex == (uNumberOfFilterSets-1) ) ? 0xfff : ((uInstructionIndex + uLocalFilterLength)/8);
    StartInstructionTemplate.bits.Link_bits11to4 = uLink >> 4;
    StartInstructionTemplate.bits.Link_bits3to0 = uLink & 0xf;
    if ( nResultRegsSelect < C3HPPC_RCE_RESULT_REGS_USE_ALL ) {
      StartInstructionTemplate.bits.Res = 1 << nResultRegsSelect;
    } else {
      StartInstructionTemplate.bits.Res = 1 << (uFilterSetIndex & 3);
    }
    StartInstructionTemplate.bits.Base = uFilterSetIndex << 12;
    g_RceInstructionArray[uInstructionIndex++] = StartInstructionTemplate.value;
  
#if 0
    cli_out("StartInstructionTemplate[%d] --> 0x%016llx\n", (uInstructionIndex-1), StartInstructionTemplate.value); 
#endif
  
    for ( nIndex = 0; nIndex < nNopCount; ++nIndex ) {
      g_RceInstructionArray[uInstructionIndex++] = NopInstructionTemplate.value;
    }

    g_RceInstructionArray[uInstructionIndex++] = PrefixInstructionTemplate.value;

    for ( nIndex = 0; nIndex < (int) uKeyLength; ++nIndex ) {
      LpmInstructionTemplate.bits.EndF = ( nIndex == ((int)(uKeyLength-1)) ) ? 1 : 0;
      uKeyBitIndex = uKeyStartIndex + nIndex;
      LpmInstructionTemplate.bits.KeyBitIndex_bits8to4 = uKeyBitIndex >> 4; 
      LpmInstructionTemplate.bits.KeyBitIndex_bits3to0 = uKeyBitIndex & 0xf; 
      g_RceInstructionArray[uInstructionIndex++] = LpmInstructionTemplate.value;
    }

  }
  
  uRegisterValue = 0;
  soc_reg_field_set( nUnit, RC_INSTCTRL_PROGRAM_CONFIGr, &uRegisterValue, ENf, 1 );
  soc_reg_field_set( nUnit, RC_INSTCTRL_PROGRAM_CONFIGr, &uRegisterValue, MAX_LENGTHf, (uMaxLength/8) );
  soc_reg_field_set( nUnit, RC_INSTCTRL_PROGRAM_CONFIGr, &uRegisterValue, BASE_ADDRf, (uProgramBaseAddress/8) );
  SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, RC_INSTCTRL_PROGRAM_CONFIGr, SOC_BLOCK_PORT(nUnit,0),
                                      nProgramNumber, uRegisterValue) );

  return 0;
}



int  c3hppc_rce_add_filter_for_lpm_exact_match( uint32 uFilterLength, uint32 uKeyLength, uint32 uFilterSetIndex,
                                                uint32 uSBlkIndex, uint32 uColumnIndex,
                                                uint32 uProgramBaseAddress, uint64 *puuPatternData )
{
  uint32 uFilterSetBitOffset, uNopCount, uStartBit, uEndBit, uPatternIndex, uFieldLength, uLocalFilterLength;
  uint64 uuNopMask;
  uint64 uuValidFilterSet = COMPILER_64_INIT(0x80000000,0x00000000);
  uint64 uuZero = COMPILER_64_INIT(0x00000000,0x00000000);

  /* RCE BUG:  Jira -- CA3-2684 */
  uLocalFilterLength = uFilterLength; 
  if ( uFilterLength == 32 && uFilterSetIndex != 0 ) {
    uFilterSetBitOffset = uProgramBaseAddress + 40 + (uLocalFilterLength * (uFilterSetIndex-1));
  } else { 
    if ( uFilterLength == 32 && uFilterSetIndex == 0 ) uLocalFilterLength = 40; 
    uFilterSetBitOffset = uProgramBaseAddress + (uLocalFilterLength * uFilterSetIndex);
  }

  uNopCount = uLocalFilterLength - uKeyLength - 2;  /* -2 comes from the valid rule/filter set and delimeter bits */

  uStartBit = 0;
  uEndBit = 0;
  uPatternIndex = 0;

  c3hppc_rce_set_pattern( uSBlkIndex, uColumnIndex, uFilterSetBitOffset, uStartBit, uEndBit,
                          uuValidFilterSet ); /* Valid Rule/Filter set */ 

  while ( uNopCount > 0 ) {
    uStartBit = uEndBit + 1;
    uFieldLength = C3HPPC_MIN(64,uNopCount);
    uEndBit = uStartBit + uFieldLength - 1;
    uuNopMask = c3hppcUtils_generate_64b_mask( 63, (64-uFieldLength) );
    c3hppc_rce_set_pattern( uSBlkIndex, uColumnIndex, uFilterSetBitOffset, uStartBit, uEndBit, 
                            uuNopMask ); 
    uNopCount -= uFieldLength;
  }

  uStartBit = uEndBit + 1;
  uEndBit = uStartBit;
  c3hppc_rce_set_pattern( uSBlkIndex, uColumnIndex, uFilterSetBitOffset, uStartBit, uEndBit,
                          uuZero );

  while ( uKeyLength > 0 ) {
    uStartBit = uEndBit + 1;
    uFieldLength = C3HPPC_MIN(64,uKeyLength);
    uEndBit = uStartBit + uFieldLength - 1;
    c3hppc_rce_set_pattern( uSBlkIndex, uColumnIndex, uFilterSetBitOffset, uStartBit, uEndBit,
                            c3hppcUtils_64bit_flip(puuPatternData[uPatternIndex++]) ); 
    uKeyLength -= uFieldLength;
  }

  return 0;
}


int c3hppc_rce_move_filter_set( int nUnit, int nProgramNumber, uint32 uFilterLength,
                                uint32 uFilterSetToMove, uint32 uNewLocation ) {
  c3hppc_rce_start_instruction_ut  StartInstructionTemplate;
  uint32 uBlockIndex, uFromFilterSetBlockIndex, uToFilterSetBlockIndex; 
  uint32 *pDmaData;
  uint32 uDmaSizeInBytes;
  uint32 uDmaBlockNum;
  uint32 uFromStartOffset, uFromEndOffset;
  uint32 uToStartOffset, uToEndOffset;
  int    nReLinkInstructionIndex;
  int rc;
  uint32 uRegisterValue, uProgramBaseAddress, uLink;

  uLink = 0;

  /* RCE BUG:  Jira -- CA3-2684 */
  uFilterLength = ( uFilterSetToMove == 0 ) ? C3HPPC_MAX(uFilterLength, 40) : uFilterLength;

  SOC_IF_ERROR_RETURN( soc_reg32_get( nUnit, RC_INSTCTRL_PROGRAM_CONFIGr, SOC_BLOCK_PORT(nUnit,0),
                                      nProgramNumber, &uRegisterValue) );
  uProgramBaseAddress = soc_reg_field_get( nUnit, RC_INSTCTRL_PROGRAM_CONFIGr, uRegisterValue, BASE_ADDRf );

  uDmaSizeInBytes = C3HPPC_RCE_DMA_BLOCK_SIZE_IN_WORDS * sizeof(uint32);
  pDmaData = (uint32 *) soc_cm_salloc( nUnit, uDmaSizeInBytes, "rce dma buffer" );


  uDmaBlockNum = uFilterLength / C3HPPC_RCE_INSTRUCTIONS_PER_DMA_BLOCK;
  uFromFilterSetBlockIndex = uProgramBaseAddress + (uFilterSetToMove * uDmaBlockNum); 
  uToFilterSetBlockIndex = (uNewLocation / C3HPPC_RCE_INSTRUCTIONS_PER_DMA_BLOCK);
  
  cli_out("\n<c3hppc_rce_move_filter_set> -- Moving FILTER SET %d from block index %d to block index %d ...\n",
          uFilterSetToMove, uFromFilterSetBlockIndex, uToFilterSetBlockIndex );

  for ( rc = 0, uBlockIndex = 0; uBlockIndex < uDmaBlockNum; uBlockIndex++ ) {
    uFromStartOffset = uFromFilterSetBlockIndex * C3HPPC_RCE_IMEM_PMEM_DMA_ENTRIES_PER_DMA_BLOCK;
    uFromEndOffset = uFromStartOffset + C3HPPC_RCE_IMEM_PMEM_DMA_ENTRIES_PER_DMA_BLOCK - 1;
    uToStartOffset = uToFilterSetBlockIndex * C3HPPC_RCE_IMEM_PMEM_DMA_ENTRIES_PER_DMA_BLOCK;
    uToEndOffset = uToStartOffset + C3HPPC_RCE_IMEM_PMEM_DMA_ENTRIES_PER_DMA_BLOCK - 1;


    rc += soc_mem_read_range(nUnit, IMEM_PMEM_DMAm, MEM_BLOCK_ANY, (int) uFromStartOffset,
                             (int) uFromEndOffset, (void *) pDmaData);

/*
uint32 *pExpectDmaData;
pExpectDmaData = (uint32 *) sal_alloc( uDmaSizeInBytes, "rce dma buffer" );
int i, nErrorCount;
nErrorCount = 0;
if ( !nErrorCount ) {
sal_memset(pExpectDmaData, 0x00, uDmaSizeInBytes);
c3hppc_rce_get_dma_block( uFromFilterSetBlockIndex, pExpectDmaData);
for ( i = 0; i < C3HPPC_RCE_DMA_BLOCK_SIZE_IN_WORDS; ++i ) {
  if ( pExpectDmaData[i] != pDmaData[i] ) {
    cli_out("<c3hppc_rce_move_filter_set> -- FROM_DMA MISCOMPARE on word[%d] offset[%d]  Actual: 0x%08x   Expect: 0x%08x \n",
            i, uFromStartOffset, pDmaData[i], pExpectDmaData[i] );
    nErrorCount = 1;
    WRITE_RC_GLOBAL_ERROR_MASKr( nUnit, 0xffffffff );
  }
}
}
sal_free( pExpectDmaData );
*/


    if ( uBlockIndex ==  0 ) {
      COMPILER_64_SET(StartInstructionTemplate.value, (pDmaData[1] & 0x7ff), pDmaData[0]); 
      StartInstructionTemplate.bits.Base |= 0x80000;
      pDmaData[0] =  COMPILER_64_LO(StartInstructionTemplate.value);
      pDmaData[1] &= 0xfffff800;
      pDmaData[1] |=  COMPILER_64_HI(StartInstructionTemplate.value);
    }


    /*    coverity[negative_returns : FALSE]    */
    rc += soc_mem_write_range(nUnit, IMEM_PMEM_DMAm, MEM_BLOCK_ANY, (int) uToStartOffset,
                              (int) uToEndOffset, (void *) pDmaData);


/*
if ( !nErrorCount ) {
rc += soc_mem_read_range(nUnit, IMEM_PMEM_DMAm, MEM_BLOCK_ANY, (int) uToStartOffset,
                         (int) uToEndOffset, (void *) pExpectDmaData);
for ( i = 0; i < C3HPPC_RCE_DMA_BLOCK_SIZE_IN_WORDS; ++i ) {
  if ( pExpectDmaData[i] != pDmaData[i] ) {
    cli_out("<c3hppc_rce_move_filter_set> -- TO_DMA MISCOMPARE on word[%d]  Actual: 0x%08x   Expect: 0x%08x \n",
            i, pExpectDmaData[i], pDmaData[i] );
    nErrorCount = 1;
    WRITE_RC_GLOBAL_ERROR_MASKr( nUnit, 0xffffffff );
  }
}
}
*/

    ++uFromFilterSetBlockIndex;
    ++uToFilterSetBlockIndex;
  }

  cli_out("\n<c3hppc_rce_move_filter_set> -- Performing RE-LINK step for move operation ...\n");

  if ( uFilterSetToMove != 0 ) {
    nReLinkInstructionIndex = uProgramBaseAddress + ((uFilterLength * (uFilterSetToMove - 1)) / C3HPPC_RCE_INSTRUCTIONS_PER_DMA_BLOCK);

    rc += soc_mem_read_range(nUnit, IMEMm, MEM_BLOCK_ANY, nReLinkInstructionIndex,
                             nReLinkInstructionIndex, (void *) pDmaData);
    COMPILER_64_SET(StartInstructionTemplate.value, (pDmaData[1] & 0x7ff), pDmaData[0]);
    uLink = (uNewLocation / C3HPPC_RCE_INSTRUCTIONS_PER_DMA_BLOCK);
    StartInstructionTemplate.bits.Link_bits11to4 = uLink >> 4;
    StartInstructionTemplate.bits.Link_bits3to0 = uLink & 0xf;
    pDmaData[0] =  COMPILER_64_LO(StartInstructionTemplate.value);
    pDmaData[1] &= 0xfffff800;
    pDmaData[1] |=  COMPILER_64_HI(StartInstructionTemplate.value);

    /*    coverity[negative_returns : FALSE]    */
    rc += soc_mem_write_range(nUnit, IMEMm, MEM_BLOCK_ANY, nReLinkInstructionIndex,
                              nReLinkInstructionIndex, (void *) pDmaData);
  } else {
    soc_reg_field_set( nUnit, RC_INSTCTRL_PROGRAM_CONFIGr, &uRegisterValue, BASE_ADDRf, (uNewLocation/C3HPPC_RCE_INSTRUCTIONS_PER_DMA_BLOCK) );
    SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, RC_INSTCTRL_PROGRAM_CONFIGr, SOC_BLOCK_PORT(nUnit,0),
                                        nProgramNumber, uRegisterValue) );
  }


  if ( rc ) {
      cli_out("<c3hppc_rce_move_filter_set>  soc_mem_write-read_range rc --> %d\n", rc);
  }

  soc_cm_sfree( nUnit, pDmaData );


  return rc;
}


void c3hppc_rce_dump_pattern_array( void )
{
  int uSBlkIndex, uColumnIndex, uColumnDataIndex;

  for ( uSBlkIndex = 0; uSBlkIndex < C3HPPC_RCE_SBLOCK_NUM; uSBlkIndex++ ) {
    for ( uColumnIndex = 0; uColumnIndex < C3HPPC_RCE_NUM_COLUMNS_PER_SBLOCK; uColumnIndex++ ) {
      for ( uColumnDataIndex = 0;
            uColumnDataIndex < C3HPPC_RCE_SBLOCK_PATTEN_COLUMN_DEPTH_IN_64B_WORDS;
            uColumnDataIndex++ ) {
      }
    }
  }

  return;
}

void c3hppc_rce_set_pattern( uint32 uSBlk, uint32 uColumn, uint32 uFilterSetBitOffset,
                             uint32 uStartBit, uint32 uEndBit, uint64 uuData)
{
  uint32 uColumnDataIndex;
  uint32 uStartBitIndex, uEndBitIndex;
  uint64 uuArrayData, uuTmp;

  uColumnDataIndex = (uFilterSetBitOffset + uStartBit) / 64;
  uStartBitIndex   = (uFilterSetBitOffset + uStartBit) % 64;
  uEndBitIndex     = (uFilterSetBitOffset + uEndBit)   % 64;

  uuTmp = c3hppcUtils_generate_64b_mask(63, (63-(uEndBit-uStartBit)));
  COMPILER_64_AND(uuData, uuTmp);
  COMPILER_64_SET(uuArrayData, COMPILER_64_HI(uuData), COMPILER_64_LO(uuData));
  COMPILER_64_SHR(uuArrayData, uStartBitIndex);
  COMPILER_64_OR(g_RcePatternArray[uSBlk][uColumn].uuColumnData[uColumnDataIndex], uuArrayData);
  if ( uEndBitIndex < uStartBitIndex ) {
    COMPILER_64_SET(uuArrayData, COMPILER_64_HI(uuData), COMPILER_64_LO(uuData));
    COMPILER_64_SHL(uuArrayData, (64 - uStartBitIndex));
    COMPILER_64_OR(g_RcePatternArray[uSBlk][uColumn].uuColumnData[uColumnDataIndex+1], uuArrayData);
  }
}


uint32 c3hppc_rce_get_pattern_bit( uint32 uSBlk, uint32 uColumn, uint32 uRow)
{
  uint32 uColumnDataIndex;
  uint32 uBitIndex;
  uint64 uuArrayData, uuTmp = COMPILER_64_INIT(0x00000000,0x00000001);

  uColumnDataIndex =  uRow / 64;
  uBitIndex   = uRow % 64;

  COMPILER_64_SET(uuArrayData,
                  COMPILER_64_HI(g_RcePatternArray[uSBlk][uColumn].uuColumnData[uColumnDataIndex]),
                  COMPILER_64_LO(g_RcePatternArray[uSBlk][uColumn].uuColumnData[uColumnDataIndex]));
  COMPILER_64_SHR(uuArrayData, (63 - uBitIndex));
  COMPILER_64_AND(uuArrayData, uuTmp);

  return ( COMPILER_64_LO(uuArrayData) );
}



void c3hppc_rce_get_dma_block( uint32 uBlockNum, uint32 *puDmaData )
{
  uint32 uCounter, uData, uInstructionIndex, uDmaDataIndex, uPatternBit;
  uint32 uSBlkIndex, uColumnIndex; 

  uDmaDataIndex = 0;
  uInstructionIndex = uBlockNum * C3HPPC_RCE_INSTRUCTIONS_PER_DMA_BLOCK;
  for ( uCounter = 0; uCounter < 2; ++uCounter ) {    /* (F) - first of 64b, (M) - middle of 64b, (R) - remainder of 64b */
    puDmaData[uDmaDataIndex++]  =  COMPILER_64_LO(g_RceInstructionArray[uInstructionIndex]);                       /*  F32 */
    puDmaData[uDmaDataIndex]    =  COMPILER_64_HI(g_RceInstructionArray[uInstructionIndex++]);                     /*  R11 */
    puDmaData[uDmaDataIndex++] |=  (COMPILER_64_LO(g_RceInstructionArray[uInstructionIndex]) & 0x001fffff) << 11;  /*  F21 */
    puDmaData[uDmaDataIndex]    =  COMPILER_64_LO(g_RceInstructionArray[uInstructionIndex]) >> 21;                 /*  R22 (11b to 32b boundary) */
    puDmaData[uDmaDataIndex]   |=  (COMPILER_64_HI(g_RceInstructionArray[uInstructionIndex++]) & 0x7ff) << 11;     /*  R22 (Next 11b after 32b boundary) */
    puDmaData[uDmaDataIndex++] |=  (COMPILER_64_LO(g_RceInstructionArray[uInstructionIndex]) & 0x000003ff) << 22;  /*  F10 */
    puDmaData[uDmaDataIndex]    =  COMPILER_64_LO(g_RceInstructionArray[uInstructionIndex]) >> 10;                 /*  M32 (22b to 32b boundary) */
    puDmaData[uDmaDataIndex++] |=  (COMPILER_64_HI(g_RceInstructionArray[uInstructionIndex]) & 0x000003ff) << 22;  /*  M32 (Next 10b after 32b boundary) */
    puDmaData[uDmaDataIndex]    =  COMPILER_64_HI(g_RceInstructionArray[uInstructionIndex++]) >> 10;               /*  R1  */
    puDmaData[uDmaDataIndex++] |=  (COMPILER_64_LO(g_RceInstructionArray[uInstructionIndex]) & 0x7fffffff) << 1;   /*  F31 */
    puDmaData[uDmaDataIndex]    =  COMPILER_64_LO(g_RceInstructionArray[uInstructionIndex]) >> 31;                 /*  R12 (1b to 32b boundary) */
    puDmaData[uDmaDataIndex++] |=  COMPILER_64_HI(g_RceInstructionArray[uInstructionIndex++]) << 1;                /*  R12 (Next 11b after 32b boundary) */
    uDmaDataIndex += 2;
  }

  uDmaDataIndex = C3HPPC_RCE_DMA_BLOCK_PATTERN_START_WORD;
  for ( uSBlkIndex = 0, uData = 0; uSBlkIndex < C3HPPC_RCE_SBLOCK_NUM; uSBlkIndex++ ) {
    for ( uColumnIndex = 0; uColumnIndex < C3HPPC_RCE_NUM_COLUMNS_PER_SBLOCK; uColumnIndex++ ) {
      for ( uInstructionIndex = uBlockNum * C3HPPC_RCE_INSTRUCTIONS_PER_DMA_BLOCK;
            uInstructionIndex < (uBlockNum * C3HPPC_RCE_INSTRUCTIONS_PER_DMA_BLOCK + C3HPPC_RCE_INSTRUCTIONS_PER_DMA_BLOCK);
            uInstructionIndex++ ) {
        uPatternBit = c3hppc_rce_get_pattern_bit( uSBlkIndex, uColumnIndex, uInstructionIndex );
        uData |= uPatternBit << ( (8*(uColumnIndex & 3)) + (uInstructionIndex & 7) );
      }
      if ( (uColumnIndex & 3) == 3 ) {
        puDmaData[uDmaDataIndex++] = uData;
        uData = 0;
      }
    }
  }


}


int c3hppc_rce_dma_image( int nUnit )
{
  uint32 uBlockIndex; 
  uint32 *pDmaData;
  uint32 uDmaSizeInBytes;
  uint32 uDmaBlockNum;
  uint32 uStartOffset, uEndOffset;
  int rc;
  

  uDmaSizeInBytes = C3HPPC_RCE_DMA_BLOCK_SIZE_IN_WORDS * sizeof(uint32);
  pDmaData = (uint32 *) soc_cm_salloc( nUnit, uDmaSizeInBytes, "rce dma buffer" );


  uDmaBlockNum = C3HPPC_RCE_NUM_DMA_BLOCKS;

  SOC_MEM_TEST_SKIP_CACHE_SET(nUnit, 0);

  for ( rc = 0, uBlockIndex = 0; uBlockIndex < uDmaBlockNum; uBlockIndex++ ) {
    sal_memset(pDmaData, 0xff, uDmaSizeInBytes);
    c3hppc_rce_get_dma_block( uBlockIndex, pDmaData);

    uStartOffset = uBlockIndex * C3HPPC_RCE_IMEM_PMEM_DMA_ENTRIES_PER_DMA_BLOCK;
    uEndOffset = uStartOffset + C3HPPC_RCE_IMEM_PMEM_DMA_ENTRIES_PER_DMA_BLOCK - 1;
    /*    coverity[negative_returns : FALSE]    */
    rc += soc_mem_write_range(nUnit, IMEM_PMEM_DMAm, MEM_BLOCK_ANY, (int) uStartOffset,
                              (int) uEndOffset, (void *) pDmaData);
  }

  if ( rc ) {
      cli_out("<c3hppc_rce_dma_image>  soc_mem_write_range rc --> %d\n", rc);
  }

  soc_cm_sfree( nUnit, pDmaData );

  return rc;
}


int c3hppc_rce_conifg_results_counters(int nUnit, int nCounterId, uint32 uRuleLo, uint32 uRuleHi ) {

  uint32 uRegisterValue;

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, RC_RESULTS_RULE_CNT_LO_CONFIGr, &uRegisterValue, RULE_CNT_LOf, uRuleLo );
  SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, RC_RESULTS_RULE_CNT_LO_CONFIGr, SOC_BLOCK_PORT(nUnit,0),
                                      nCounterId, uRegisterValue) );

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, RC_RESULTS_RULE_CNT_HI_CONFIGr, &uRegisterValue, RULE_CNT_HIf, uRuleHi );
  SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, RC_RESULTS_RULE_CNT_HI_CONFIGr, SOC_BLOCK_PORT(nUnit,0),
                                      nCounterId, uRegisterValue) );


  return 0;
}


int c3hppc_rce_read_results_counters(int nUnit ) {
  int nCounterId;
  uint32 uRegisterValue;

  for ( nCounterId = 0; nCounterId < C3HPPC_RCE_RESULTS_COUNTER_NUM; ++nCounterId ) {
    SOC_IF_ERROR_RETURN( soc_reg32_get( nUnit, RC_RESULTS_RULE_CNTr, SOC_BLOCK_PORT(nUnit,0),
                                        nCounterId, &uRegisterValue ) );
    COMPILER_64_ADD_32(g_auuResultsRuleCounters[nCounterId], uRegisterValue);
  }


  return 0;
}

uint64 c3hppc_rce_get_results_counter(int nCounterId ) {

  return g_auuResultsRuleCounters[nCounterId];

}


#endif   /* #ifdef BCM_CALADAN3_SUPPORT */
