/*
 * $Id: c3hppc_ocm.c,v 1.13.122.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    c3hppc_ocm.c
 * Purpose: Caladan3 OCM test driver
 * Requires:
 */


#include <shared/bsl.h>

#include <soc/defs.h>
#include <soc/mem.h>

#ifdef BCM_CALADAN3_SUPPORT

#include <appl/test/caladan3/c3hppc_ocm.h>


static int g_anErrorRegisters[] = { OC_MISC_ERRORr,
                                    OC_LRP_PORT0_MAPPING_ERRORr,
                                    OC_LRP_PORT1_MAPPING_ERRORr,
                                    OC_LRP_PORT2_MAPPING_ERRORr,
                                    OC_LRP_PORT3_MAPPING_ERRORr,
                                    OC_LRP_PORT4_MAPPING_ERRORr,
                                    OC_LRP_PORT5_MAPPING_ERRORr,
                                    OC_LRP_PORT6_MAPPING_ERRORr,
                                    OC_LRP_PORT7_MAPPING_ERRORr,
                                    OC_LRP_PORT8_MAPPING_ERRORr,
                                    OC_LRP_PORT9_MAPPING_ERRORr,
                                    OC_LRP_BUBBLE_PORT_MAPPING_ERRORr,
                                    OC_CMU_PORT0_MAPPING_ERRORr,
                                    OC_CMU_PORT1_MAPPING_ERRORr,
                                    OC_COP0_PORT_MAPPING_ERRORr,
                                    OC_COP1_PORT_MAPPING_ERRORr,
                                    OC_SEGMENT_TABLE_ECC_ERRORr,
                                    OC_BLOCK_TABLE_PARITY_ERRORr,
                                    OC_MEMORY_ERRORr,
                                    OC_LRP_PORT0_MAPPING_ERROR_STATUS0r,
                                    OC_LRP_PORT1_MAPPING_ERROR_STATUS0r,
                                    OC_LRP_PORT2_MAPPING_ERROR_STATUS0r,
                                    OC_LRP_PORT3_MAPPING_ERROR_STATUS0r,
                                    OC_LRP_PORT4_MAPPING_ERROR_STATUS0r,
                                    OC_LRP_PORT5_MAPPING_ERROR_STATUS0r,
                                    OC_LRP_PORT6_MAPPING_ERROR_STATUS0r,
                                    OC_LRP_PORT7_MAPPING_ERROR_STATUS0r,
                                    OC_LRP_PORT8_MAPPING_ERROR_STATUS0r,
                                    OC_LRP_PORT9_MAPPING_ERROR_STATUS0r,
                                    OC_LRP_PORT0_MAPPING_ERROR_STATUS1r,
                                    OC_LRP_PORT1_MAPPING_ERROR_STATUS1r,
                                    OC_LRP_PORT2_MAPPING_ERROR_STATUS1r,
                                    OC_LRP_PORT3_MAPPING_ERROR_STATUS1r,
                                    OC_LRP_PORT4_MAPPING_ERROR_STATUS1r,
                                    OC_LRP_PORT5_MAPPING_ERROR_STATUS1r,
                                    OC_LRP_PORT6_MAPPING_ERROR_STATUS1r,
                                    OC_LRP_PORT7_MAPPING_ERROR_STATUS1r,
                                    OC_LRP_PORT8_MAPPING_ERROR_STATUS1r,
                                    OC_LRP_PORT9_MAPPING_ERROR_STATUS1r,
                                    OC_LRP_BUBBLE_PORT_MAPPING_ERROR_STATUS0r,
                                    OC_LRP_BUBBLE_PORT_MAPPING_ERROR_STATUS1r,
                                    OC_CMU_PORT0_MAPPING_ERROR_STATUSr,
                                    OC_CMU_PORT1_MAPPING_ERROR_STATUSr,
                                    OC_COP0_PORT_MAPPING_ERROR_STATUSr,
                                    OC_COP1_PORT_MAPPING_ERROR_STATUSr,
                                    OC_SEGMENT_TABLE_ECC_ERROR_STATUS0r,
                                    OC_SEGMENT_TABLE_ECC_ERROR_STATUS1r,
                                    OC_SEGMENT_TABLE_ECC_ERROR_STATUS2r,
                                    OC_BLOCK_TABLE_PARITY_ERROR_STATUSr,
                                    OC_MEMORY_ERROR_STATUS0r,
                                    OC_MEMORY_ERROR_STATUS1r,
                                    OC_MEMORY_ERROR_STATUS2r
                                  };
static int g_nErrorRegistersCount = COUNTOF(g_anErrorRegisters);

static int g_anErrorMaskRegisters[] = { OC_MISC_ERROR_MASKr,
                                        OC_LRP_PORT0_MAPPING_ERROR_MASKr,
                                        OC_LRP_PORT1_MAPPING_ERROR_MASKr,
                                        OC_LRP_PORT2_MAPPING_ERROR_MASKr,
                                        OC_LRP_PORT3_MAPPING_ERROR_MASKr,
                                        OC_LRP_PORT4_MAPPING_ERROR_MASKr,
                                        OC_LRP_PORT5_MAPPING_ERROR_MASKr,
                                        OC_LRP_PORT6_MAPPING_ERROR_MASKr,
                                        OC_LRP_PORT7_MAPPING_ERROR_MASKr,
                                        OC_LRP_PORT8_MAPPING_ERROR_MASKr,
                                        OC_LRP_PORT9_MAPPING_ERROR_MASKr,
                                        OC_LRP_BUBBLE_PORT_MAPPING_ERROR_MASKr,
                                        OC_CMU_PORT0_MAPPING_ERROR_MASKr,
                                        OC_CMU_PORT1_MAPPING_ERROR_MASKr,
                                        OC_COP0_PORT_MAPPING_ERROR_MASKr,
                                        OC_COP1_PORT_MAPPING_ERROR_MASKr,
                                        OC_SEGMENT_TABLE_ECC_ERROR_MASKr,
                                        OC_BLOCK_TABLE_PARITY_ERROR_MASKr,
                                        OC_MEMORY_ERROR_MASKr
                                      };
static int g_nErrorMaskRegistersCount = COUNTOF(g_anErrorMaskRegisters);



static int g_anLrpPort2OcmPortMappingTable[] = {0, 1, 2, 3, 4, 5, 9, 10, 11, 12};
static int g_anCopPort2OcmPortMappingTable[] = {7, 14};
static int g_anCmuPort2OcmPortMappingTable[] = {6, 13};
static int g_anBubblePort2OcmPortMappingTable[] = {8};

static 
soc_mem_t oc_port_segment_mem[] = {
    OC_LRP_PORT0_SEGMENTm, OC_LRP_PORT1_SEGMENTm, OC_LRP_PORT2_SEGMENTm,
    OC_LRP_PORT3_SEGMENTm, OC_LRP_PORT4_SEGMENTm, OC_LRP_PORT5_SEGMENTm,
    C3HPPC_NO_SEGMENT_TABLE, C3HPPC_NO_SEGMENT_TABLE, OC_LRP_BUBBLE_PORT_SEGMENTm,
    OC_LRP_PORT6_SEGMENTm, OC_LRP_PORT7_SEGMENTm, OC_LRP_PORT8_SEGMENTm,
    OC_LRP_PORT9_SEGMENTm, C3HPPC_NO_SEGMENT_TABLE, C3HPPC_NO_SEGMENT_TABLE
};

static 
soc_mem_t oc_port_block_mem[] = {
    OC_LRP_PORT0_BLOCKm, OC_LRP_PORT1_BLOCKm, OC_LRP_PORT2_BLOCKm,
    OC_LRP_PORT3_BLOCKm, OC_LRP_PORT4_BLOCKm, OC_LRP_PORT5_BLOCKm,
    OC_CMU_PORT0_BLOCKm, OC_COP0_PORT_BLOCKm, OC_LRP_BUBBLE_PORT_BLOCKm,
    OC_LRP_PORT6_BLOCKm, OC_LRP_PORT7_BLOCKm, OC_LRP_PORT8_BLOCKm,
    OC_LRP_PORT9_BLOCKm, OC_CMU_PORT1_BLOCKm, OC_COP1_PORT_BLOCKm
};


int c3hppc_ocm_hw_init( int nUnit, c3hppc_ocm_control_info_t *pC3OcmControlInfo ) {

/*  uint32 u32bRegisterValue, uFieldValue; */
  int rc;
  uint32 uRegisterValue;
  sal_time_t TimeStamp;


  READ_OC_CONFIGr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, OC_CONFIGr, &uRegisterValue, SOFT_RESET_Nf, 1 );
  WRITE_OC_CONFIGr( nUnit, uRegisterValue );

  WRITE_OC_MEMORY_INIT_DATA0r( nUnit, 0 );
  WRITE_OC_MEMORY_INIT_DATA1r( nUnit, 0 );
  /* only used when proc raw is enabled */
  WRITE_OC_MEMORY_INIT_DATA2r( nUnit, 0 );

  WRITE_OC_MEMORY_INIT_ENABLE0r( nUnit, 0xffffffff );
  WRITE_OC_MEMORY_INIT_ENABLE1r( nUnit, 0xffffffff );
  WRITE_OC_MEMORY_INIT_ENABLE2r( nUnit, 0xffffffff );
  WRITE_OC_MEMORY_INIT_ENABLE3r( nUnit, 0xffffffff );

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, OC_MEMORY_INIT_CONTROLr, &uRegisterValue, INIT_ERR_PROTf, C3HPPC_ERROR_PROTECTION__ECC );
  soc_reg_field_set( nUnit, OC_MEMORY_INIT_CONTROLr, &uRegisterValue, INIT_ENABLEf, 1 );
  soc_reg_field_set( nUnit, OC_MEMORY_INIT_CONTROLr, &uRegisterValue, INIT_GOf, 1 );
  WRITE_OC_MEMORY_INIT_CONTROLr( nUnit, uRegisterValue);

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, OC_SEGMENT_TABLE_INIT_CONTROLr, &uRegisterValue, INIT_GOf, 1 );
  WRITE_OC_SEGMENT_TABLE_INIT_CONTROLr( nUnit, uRegisterValue);

  
  if ( c3hppcUtils_poll_field( nUnit, REG_PORT_ANY, OC_SEGMENT_TABLE_INIT_CONTROLr, INIT_DONEf, 1, 100, 0, &TimeStamp ) ) {
    cli_out("<c3hppc_ocm_hw_init> -- OC_SEGMENT_TABLE_INIT_CONTROL \"INIT_DONE\" event TIMEOUT!!!\n");
    return -1;
  }
  
  if ( c3hppcUtils_poll_field( nUnit, REG_PORT_ANY, OC_MEMORY_INIT_CONTROLr, INIT_DONEf, 1, 100, 0, &TimeStamp ) ) {
    cli_out("<c3hppc_ocm_hw_init> -- OC_MEMORY_INIT_CONTROL \"INIT_DONE\" event TIMEOUT!!!\n");
    return -1;
  }

  rc = c3hppc_ocm_port_config( nUnit, pC3OcmControlInfo->pOcmPortInfo );
  
  return rc;
}


int c3hppc_ocm_map_lrp2ocm_port(int nLrpPort) {
  return g_anLrpPort2OcmPortMappingTable[nLrpPort];
}


int c3hppc_ocm_map_cop2ocm_port(int nCopPort) {
  return g_anCopPort2OcmPortMappingTable[nCopPort];
}


int c3hppc_ocm_map_cmu2ocm_port(int nCmuPort) {
  return g_anCmuPort2OcmPortMappingTable[nCmuPort];
}


int c3hppc_ocm_map_bubble2ocm_port(int nBubblePort) {
  return g_anBubblePort2OcmPortMappingTable[nBubblePort];
}


int c3hppc_ocm_port_config( int nUnit, c3hppc_ocm_port_info_t *pOcmPortInfo ) {

  int nPort, nSegment, nSegmentNum;
  uint32 uSegmentTransferSize;
  c3hppc_ocm_port_info_t *pPortInfo;
  
  for ( nPort = 0; nPort < C3HPPC_NUM_OF_OCM_PORTS; ++nPort ) {
    pPortInfo = pOcmPortInfo + nPort;
    if ( pPortInfo->bValid ) {
      nSegmentNum = ( pPortInfo->uSegmentTransferSize == C3HPPC_OCM_ALL_TRANSFER_SIZES ) ? C3HPPC_OCM_NUM_OF_TRANSFER_SIZES : 1; 
      for ( nSegment = 0; nSegment < nSegmentNum; ++nSegment ) {
        uSegmentTransferSize = ( pPortInfo->uSegmentTransferSize == C3HPPC_OCM_ALL_TRANSFER_SIZES ) ? nSegment : 
                                                                                  pPortInfo->uSegmentTransferSize;
        if ( oc_port_segment_mem[nPort] != C3HPPC_NO_SEGMENT_TABLE ) {
          if ( c3hppc_ocm_port_program_segment_table(nUnit, nPort,
                                                     pPortInfo->nStartingSegment + nSegment, uSegmentTransferSize,
                                                     pPortInfo->uSegmentBase, pPortInfo->uSegmentLimit, 
                                                     pPortInfo->bSegmentProtected) ) {
          }
        }
        if ( uSegmentTransferSize == C3HPPC_DATUM_SIZE_QUADWORD ||
             pPortInfo->uSegmentTransferSize != C3HPPC_OCM_ALL_TRANSFER_SIZES ) {
          if ( c3hppc_ocm_port_program_port_table(nUnit, nPort,
                                                  pPortInfo->uSegmentBase, pPortInfo->uSegmentLimit, uSegmentTransferSize,
                                                  pPortInfo->nStartingPhysicalBlock) ) {
          }
        }
      }
    }
  }

  return 0;
}


int c3hppc_ocm_port_program_segment_table(int nUnit, int nOcmPort, int nSegment,
                                          uint32 uSegmentTransferSize, uint32 uSegmentBase,
                                          uint32 uSegmentLimit, char bSegmentProtected)
{
  uint32 uFieldValue, auSegmentTableEntry[4];

  SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, oc_port_segment_mem[nOcmPort],
                                    MEM_BLOCK_ANY, nSegment, auSegmentTableEntry) );

  soc_mem_field_set( nUnit, oc_port_segment_mem[nOcmPort],
                     auSegmentTableEntry, SEGMENT_BASEf, &uSegmentBase );
  soc_mem_field_set( nUnit, oc_port_segment_mem[nOcmPort],
                     auSegmentTableEntry, SEGMENT_SIZEf, &uSegmentTransferSize );
  soc_mem_field_set( nUnit, oc_port_segment_mem[nOcmPort],
                     auSegmentTableEntry, SEGMENT_LIMITf, &uSegmentLimit );
  if ( bSegmentProtected ) {
    uFieldValue = ( uSegmentTransferSize == C3HPPC_DATUM_SIZE_QUADWORD ) ? C3HPPC_ERROR_PROTECTION__ECC :
                                                                           C3HPPC_ERROR_PROTECTION__PARITY; 
    soc_mem_field_set( nUnit, oc_port_segment_mem[nOcmPort],
                       auSegmentTableEntry, SEGMENT_ERR_PROTf, &uFieldValue );
  }

  SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, oc_port_segment_mem[nOcmPort],
                                     MEM_BLOCK_ANY, nSegment, auSegmentTableEntry) );

  return 0;
}


int c3hppc_ocm_port_modify_segment_error_protection(int nUnit, int nOcmPort, int nSegment, uint32 uProtectionScheme)
{
  uint32 uFieldValue, auSegmentTableEntry[4];

  SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, oc_port_segment_mem[nOcmPort],
                                    MEM_BLOCK_ANY, nSegment, auSegmentTableEntry) );

  uFieldValue = uProtectionScheme;
  soc_mem_field_set( nUnit, oc_port_segment_mem[nOcmPort],
                     auSegmentTableEntry, SEGMENT_ERR_PROTf, &uFieldValue );

  SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, oc_port_segment_mem[nOcmPort],
                                     MEM_BLOCK_ANY, nSegment, auSegmentTableEntry) );

  return 0;
}

int c3hppc_ocm_port_program_port_table(int nUnit, int nOcmPort, uint32 uSegmentBase,
                                       uint32 uSegmentLimit, uint32 uSegmentTransferSize,
                                       int nStartingPhysicalBlock)
{

  int nShift;
  uint32 uLogicalAddressInRows, uMaxLogicalAddressInRows;
  int nTableIndex;
  uint32 uFieldValue, auPortTableEntry[4];

  nShift = C3HPPC_OCM_NUM_OF_TRANSFER_SIZES - uSegmentTransferSize - 1;
  uMaxLogicalAddressInRows = uSegmentBase + ((uSegmentLimit+1) >> nShift); 

  for ( uLogicalAddressInRows = uSegmentBase; 
        uLogicalAddressInRows < uMaxLogicalAddressInRows;
        uLogicalAddressInRows += C3HPPC_OCM_MAX_PHY_BLK_SIZE_IN_ROWS ) {

    nTableIndex = (int) (uLogicalAddressInRows >> C3HPPC_LOGICAL_TO_PHYSICAL_SHIFT);

    SOC_IF_ERROR_RETURN(soc_mem_read( nUnit, oc_port_block_mem[nOcmPort],
                                      MEM_BLOCK_ANY, nTableIndex, auPortTableEntry) );

    soc_mem_field_set( nUnit, oc_port_block_mem[nOcmPort],
                       auPortTableEntry, PHYSICAL_BLOCKf, ((uint32 *) &nStartingPhysicalBlock) );
    uFieldValue = 1;
    soc_mem_field_set( nUnit, oc_port_block_mem[nOcmPort],
                       auPortTableEntry, VALIDf, &uFieldValue);

    SOC_IF_ERROR_RETURN(soc_mem_write( nUnit, oc_port_block_mem[nOcmPort],
                                       MEM_BLOCK_ANY, nTableIndex, auPortTableEntry) );

    ++nStartingPhysicalBlock;
  }

  return 0;
}


int c3hppc_ocm_mem_read_write( int nUnit, int nOcmPort, int nSegment, uint32 uOffset, 
                               uint8 bWrite, uint32 *puEntryData )
{
  uint32 u32bRegisterValue = 0;

  /* treat as logical access */
  soc_reg_field_set(nUnit, OC_CONFIGr, &u32bRegisterValue, SOFT_RESET_Nf, 1);
  soc_reg_field_set(nUnit, OC_CONFIGr, &u32bRegisterValue, PROC_PORT_SEGMENTf, (uint32) nSegment);
  soc_reg_field_set(nUnit, OC_CONFIGr, &u32bRegisterValue, PROC_PORT_IDf, (uint32) nOcmPort);
  WRITE_OC_CONFIGr(nUnit, u32bRegisterValue);

  if ( bWrite ) {
    SOC_IF_ERROR_RETURN(WRITE_OC_MEMORYm(nUnit, MEM_BLOCK_ANY, uOffset, puEntryData));
  } else {
    SOC_IF_ERROR_RETURN(READ_OC_MEMORYm(nUnit, MEM_BLOCK_ANY, uOffset, puEntryData));
  }

  return 0;
}


int c3hppc_ocm_dma_read_write( int nUnit, int nOcmPort, int nSegment, uint32 uStartOffset,
                               uint32 uEndOffset, uint8 bWrite, uint32 *puDmaData )
{
  int rc;
  uint32 u32bRegisterValue = 0;

  /* treat as logical access */
  soc_reg_field_set(nUnit, OC_CONFIGr, &u32bRegisterValue, SOFT_RESET_Nf, 1);
  soc_reg_field_set(nUnit, OC_CONFIGr, &u32bRegisterValue, DMA_PORT_SEGMENTf, (uint32) nSegment);
  soc_reg_field_set(nUnit, OC_CONFIGr, &u32bRegisterValue, DMA_PORT_IDf, (uint32) nOcmPort);
  WRITE_OC_CONFIGr(nUnit, u32bRegisterValue);

#if 0
  cli_out("<c3hppc_ocm_dma_read_write>  u32bRegisterValue --> 0x%08x  nSegment %d nOcmPort %d\n", u32bRegisterValue, nSegment, nOcmPort);
#endif

  if ( bWrite ) {
    /*    coverity[negative_returns : FALSE]    */
    rc = soc_mem_write_range(nUnit, OC_MEMORYm, MEM_BLOCK_ANY, (int) uStartOffset,
                             (int) uEndOffset, (void *) puDmaData);
    if ( rc ) {
        cli_out("<c3hppc_ocm_dma_read_write>  soc_mem_write_range rc --> %d\n", rc);
    }
  } else {
    rc = soc_mem_read_range(nUnit, OC_MEMORYm, MEM_BLOCK_ANY, (int) uStartOffset,
                            (int) uEndOffset, (void *) puDmaData);
    if ( rc ) {
        cli_out("<c3hppc_ocm_dma_read_write>  soc_mem_read_range rc --> %d\n", rc);
    }
  }

  return rc;
}





int c3hppc_ocm_hw_cleanup( int nUnit ) {

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



int c3hppc_ocm_display_error_state( int nUnit ) {

  int rc, nIndex;

  for ( rc = 0, nIndex = 0; nIndex < g_nErrorRegistersCount; ++nIndex ) {
    rc += c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,0), g_anErrorRegisters[nIndex] );
  }

  return rc;
}


#endif   /* #ifdef BCM_CALADAN3_SUPPORT */
