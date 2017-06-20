/*
 * $Id: c3hppc_cmu.c,v 1.6 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    c3hppc_cmu.c
 * Purpose: Caladan3 CMU test driver
 * Requires:
 */


#include <shared/bsl.h>

#include <soc/defs.h>

#ifdef BCM_CALADAN3_SUPPORT

#include <appl/test/caladan3/c3hppc_cmu.h>
#include <appl/test/caladan3/c3hppc_ocm.h>


static int g_anErrorRegisters[] = { CM_DISABLED_SEGMENT_ERRORr,
                                    CM_SEGMENT_LIMIT_ERRORr,
                                    CM_ERROR_STATUSr,
                                    CM_ECC_ERRORr,
                                    CM_ECC_STATUS0r,
                                    CM_ECC_STATUS1r,
                                    CM_ECC_STATUS2r,
                                    CM_ECC_STATUS3r,
                                    CM_ECC_STATUS4r,
                                    CM_ECC_STATUS5r,
                                    CM_ECC_STATUS6r
                                  };
static int g_nErrorRegistersCount = COUNTOF(g_anErrorRegisters);

static int g_anErrorMaskRegisters[] = { CM_DISABLED_SEGMENT_ERROR_MASKr,
                                        CM_SEGMENT_LIMIT_ERROR_MASKr,
                                        CM_ERROR_STATUS_MASKr,
                                        CM_ECC_ERROR_MASKr
                                      };
static int g_nErrorMaskRegistersCount = COUNTOF(g_anErrorMaskRegisters);



int c3hppc_cmu_hw_init( int nUnit, c3hppc_cmu_control_info_t *pC3CmuControlInfo ) {

  uint32 uRegisterValue;

  /*
   * Take block out of soft reset
  */
  READ_CM_CONTROL_REGISTERr( nUnit, &uRegisterValue );
  soc_reg_field_set( nUnit, CM_CONTROL_REGISTERr, &uRegisterValue, CM_RESET_Nf, 1 );
  WRITE_CM_CONTROL_REGISTERr( nUnit, uRegisterValue );

  WRITE_CM_STACE_LFSR_SEEDr( nUnit, pC3CmuControlInfo->uLFSRseed );

  if ( c3hppc_cmu_segments_config( nUnit, pC3CmuControlInfo->pCmuSegmentInfo ) ) {
    return -1;
  }

  return 0;
}


int c3hppc_cmu_segments_config( int nUnit, c3hppc_cmu_segment_info_t *pCmuSegmentInfo ) {

  int nSegment, nOcmPort;
  c3hppc_cmu_segment_info_t *pSegmentInfo;

  for ( nSegment = 0; nSegment < C3HPPC_CMU_SEGMENT_NUM; ++nSegment ) {
    pSegmentInfo = pCmuSegmentInfo + nSegment;
    if ( pSegmentInfo->bValid ) {
      if ( c3hppc_cmu_program_segment_table(nUnit, nSegment, pSegmentInfo) ) {
      }
      nOcmPort = c3hppc_ocm_map_cmu2ocm_port( pSegmentInfo->uSegmentPort );
      c3hppc_ocm_port_program_port_table( nUnit, nOcmPort, pSegmentInfo->uSegmentOcmBase,
                                          pSegmentInfo->uSegmentLimit, C3HPPC_DATUM_SIZE_QUADWORD,
                                          pSegmentInfo->nStartingPhysicalBlock );
    }
  }

  return 0;
}


int c3hppc_cmu_segments_enable( int nUnit, c3hppc_cmu_segment_info_t *pCmuSegmentInfo ) {

  int nSegment;
  c3hppc_cmu_segment_info_t *pSegmentInfo;

  for ( nSegment = 0; nSegment < C3HPPC_CMU_SEGMENT_NUM; ++nSegment ) {
    pSegmentInfo = pCmuSegmentInfo + nSegment;
    if ( pSegmentInfo->bValid ) {
      if ( c3hppc_cmu_program_segment_enable( nUnit, nSegment ) ) {
      }
    }
  }

  return 0;
}


int c3hppc_cmu_segments_ejection_enable( int nUnit, c3hppc_cmu_segment_info_t *pCmuSegmentInfo ) {

  int nSegment;
  c3hppc_cmu_segment_info_t *pSegmentInfo;

  for ( nSegment = 0; nSegment < C3HPPC_CMU_SEGMENT_NUM; ++nSegment ) {
    pSegmentInfo = pCmuSegmentInfo + nSegment;
    if ( pSegmentInfo->bValid ) {
      if ( c3hppc_cmu_program_segment_ejection_enable( nUnit, nSegment ) ) {
      }
    }
  }

  return 0;
}


int c3hppc_cmu_segments_flush( int nUnit, c3hppc_cmu_segment_info_t *pCmuSegmentInfo ) {

  int nSegment, rc;
  c3hppc_cmu_segment_info_t *pSegmentInfo;
  int nTimeOut;
  uint32 uCmicValidEntryNum, uCmuValidEjectEntry;
  uint32 uSegmentEnables, uRegisterValue;

  SOC_IF_ERROR_RETURN( soc_reg32_get( nUnit, CM_SEGMENT_ENABLEr, SOC_BLOCK_PORT(nUnit,0),
                                      0, &uSegmentEnables ) );

  for ( rc = 0, nSegment = 0; nSegment < C3HPPC_CMU_SEGMENT_NUM; ++nSegment ) {
    pSegmentInfo = pCmuSegmentInfo + nSegment;
    if ( pSegmentInfo->bValid && (uSegmentEnables & (1 << nSegment)) ) {
      rc += c3hppc_cmu_segment_flush( nUnit, pSegmentInfo );
    }
  }

  if ( uSegmentEnables ) {
    nTimeOut = 100;
    while ( --nTimeOut ) {
      sal_usleep(1000000);
      READ_CMIC_CMC0_FIFO_CH0_RD_DMA_NUM_OF_ENTRIES_VALID_IN_HOSTMEMr( nUnit, &uCmicValidEntryNum );
      READ_CM_INTERRUPT_STATUSr( nUnit, &uRegisterValue );
      uCmuValidEjectEntry = soc_reg_field_get( nUnit, CM_INTERRUPT_STATUSr, uRegisterValue, EJECTION_FIFO_READYf ); 
      if ( uCmuValidEjectEntry ) {
        uRegisterValue = 0;
        soc_reg_field_set(nUnit, CM_INTERRUPT_STATUSr, &uRegisterValue, EJECTION_FIFO_READYf, 1);
        WRITE_CM_INTERRUPT_STATUSr( nUnit, uRegisterValue );
      }
      if ( (uCmicValidEntryNum == 0 && uCmuValidEjectEntry == 0) || nTimeOut == 1) break;
    }
    if ( nTimeOut == 1 ) {
      cli_out("\n<c3hppc_cmu_segments_flush> -- ERROR: Host counter ring processing hit a TIMEOUT condition!! \n");
      rc += 1;
    } else {
      cli_out("\n<c3hppc_cmu_segments_flush> -- Host counter ring processing is complete! \n");
    }
  }

  return rc;
}


int c3hppc_cmu_program_segment_table( int nUnit, int nSegment,
                                      c3hppc_cmu_segment_info_t *pCmuSegmentInfo )
{
  uint32 uRegisterValue;

  uRegisterValue = 0;
  soc_reg_field_set( nUnit, CM_SEGMENT_TABLE_CONFIGr, &uRegisterValue, PORTf, pCmuSegmentInfo->uSegmentPort );
  soc_reg_field_set( nUnit, CM_SEGMENT_TABLE_CONFIGr, &uRegisterValue, Tf, pCmuSegmentInfo->uSegmentType );
  SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, CM_SEGMENT_TABLE_CONFIGr, SOC_BLOCK_PORT(nUnit,0), 
                                      nSegment, uRegisterValue) );

  SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, CM_SEGMENT_TABLE_OCM_BASEr, SOC_BLOCK_PORT(nUnit,0), 
                                      nSegment, pCmuSegmentInfo->uSegmentOcmBase) );

  SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, CM_SEGMENT_TABLE_LIMITr, SOC_BLOCK_PORT(nUnit,0), 
                                      nSegment, pCmuSegmentInfo->uSegmentLimit) );

  SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, CM_SEGMENT_TABLE_PCI_BASEr, SOC_BLOCK_PORT(nUnit,0), 
                                      nSegment, (uint32) pCmuSegmentInfo->pSegmentPciBase) );

  return 0;
}


int c3hppc_cmu_program_segment_enable( int nUnit, int nSegment )
{
  uint32 uRegisterValue;

  SOC_IF_ERROR_RETURN( soc_reg32_get( nUnit, CM_SEGMENT_ENABLEr, SOC_BLOCK_PORT(nUnit,0),
                                      0, &uRegisterValue ) );
  uRegisterValue |= (1 << nSegment);
  SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, CM_SEGMENT_ENABLEr, SOC_BLOCK_PORT(nUnit,0), 
                                      0, uRegisterValue) );

  SOC_IF_ERROR_RETURN( soc_reg32_get( nUnit, CM_BACKGROUND_EJECT_ENABLEr, SOC_BLOCK_PORT(nUnit,0),
                                      0, &uRegisterValue ) );
  uRegisterValue |= (1 << nSegment);
  SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, CM_BACKGROUND_EJECT_ENABLEr, SOC_BLOCK_PORT(nUnit,0), 
                                      0, uRegisterValue) );

  return 0;
}


int c3hppc_cmu_program_segment_ejection_enable( int nUnit, int nSegment )
{
  uint32 uRegisterValue;

  SOC_IF_ERROR_RETURN( soc_reg32_get( nUnit, CM_BACKGROUND_EJECT_ENABLEr, SOC_BLOCK_PORT(nUnit,0),
                                      0, &uRegisterValue ) );
  uRegisterValue |= (1 << nSegment);
  SOC_IF_ERROR_RETURN( soc_reg32_set( nUnit, CM_BACKGROUND_EJECT_ENABLEr, SOC_BLOCK_PORT(nUnit,0), 
                                      0, uRegisterValue) );

  return 0;
}


int c3hppc_cmu_segment_flush( int nUnit, c3hppc_cmu_segment_info_t *pCmuSegmentInfo )
{
  int rc;
  uint32 uRegisterValue;
  int nTimeOut;
  sal_time_t TimeStamp;

  rc = 0;

  /* This rate adjustment will probably need attention when NOT emulation */
  SOC_IF_ERROR_RETURN( WRITE_CM_MANUAL_EJECT_RATEr( nUnit, 1 ) );

  cli_out("\n<c3hppc_cmu_segment_flush> -- Flushing CMU Segment %d \n", pCmuSegmentInfo->uSegment );

  SOC_IF_ERROR_RETURN( WRITE_CM_MANUAL_EJECT_LIMITr( nUnit, pCmuSegmentInfo->uSegmentLimit ) );

  uRegisterValue = 0;
  soc_reg_field_set(nUnit, CM_MANUAL_EJECT_CONFIGr, &uRegisterValue, ENABLEf, 1);
  soc_reg_field_set(nUnit, CM_MANUAL_EJECT_CONFIGr, &uRegisterValue, GOf, 1);
  soc_reg_field_set(nUnit, CM_MANUAL_EJECT_CONFIGr, &uRegisterValue, SEGMENTf, pCmuSegmentInfo->uSegment );
  soc_reg_field_set(nUnit, CM_MANUAL_EJECT_CONFIGr, &uRegisterValue, START_IDf, 0 );
  SOC_IF_ERROR_RETURN( WRITE_CM_MANUAL_EJECT_CONFIGr( nUnit, uRegisterValue ) );

  nTimeOut = 10000 * (1 + ( (pCmuSegmentInfo->uSegmentLimit+1) / 0x10000) );
  if ( c3hppcUtils_poll_field( nUnit, REG_PORT_ANY, CM_INTERRUPT_STATUSr, MANUAL_EJECT_DONEf, 1, nTimeOut, 1, &TimeStamp) ) {
    cli_out("<c3hppc_test_done> -- ERROR: CMU Segment %d flush operation TIMEOUT!!!\n", pCmuSegmentInfo->uSegment );
    rc = 1;
  }

  return rc;
}




int c3hppc_cmu_hw_cleanup( int nUnit ) {

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



int c3hppc_cmu_display_error_state( int nUnit ) {

  int rc, nIndex;

  for ( rc = 0, nIndex = 0; nIndex < g_nErrorRegistersCount; ++nIndex ) {
    rc += c3hppcUtils_display_register_notzero( nUnit, SOC_BLOCK_PORT(nUnit,0), g_anErrorRegisters[nIndex] );
  }

  return rc;
}

#endif   /* #ifdef BCM_CALADAN3_SUPPORT */
