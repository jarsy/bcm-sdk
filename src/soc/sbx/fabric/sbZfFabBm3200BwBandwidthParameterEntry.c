/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <shared/bsl.h>

#include "sbTypes.h"
#include "sbZfFabBm3200BwBandwidthParameterEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm3200BwBandwidthParameterEntry_Pack(sbZfFabBm3200BwBandwidthParameterEntry_t *pFrom,
                                            uint8 *pToData,
                                            uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_BANDWIDTH_PARAMETER_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nGamma */
  (pToData)[0] |= ((pFrom)->m_nGamma) & 0xFF;

  /* Pack Member: m_nSigma */
  (pToData)[3] |= ((pFrom)->m_nSigma) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nSigma >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nSigma >> 16) &0xFF;
#else
  int i;
  int size = SB_ZF_FAB_BM3200_BANDWIDTH_PARAMETER_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nGamma */
  (pToData)[3] |= ((pFrom)->m_nGamma) & 0xFF;

  /* Pack Member: m_nSigma */
  (pToData)[0] |= ((pFrom)->m_nSigma) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nSigma >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nSigma >> 16) &0xFF;
#endif

  return SB_ZF_FAB_BM3200_BANDWIDTH_PARAMETER_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200BwBandwidthParameterEntry_Unpack(sbZfFabBm3200BwBandwidthParameterEntry_t *pToStruct,
                                              uint8 *pFromData,
                                              uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nGamma */
  (pToStruct)->m_nGamma =  (uint32)  (pFromData)[0] ;

  /* Unpack Member: m_nSigma */
  (pToStruct)->m_nSigma =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nSigma |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_nSigma |=  (uint32)  (pFromData)[1] << 16;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nGamma */
  (pToStruct)->m_nGamma =  (uint32)  (pFromData)[3] ;

  /* Unpack Member: m_nSigma */
  (pToStruct)->m_nSigma =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nSigma |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_nSigma |=  (uint32)  (pFromData)[2] << 16;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200BwBandwidthParameterEntry_InitInstance(sbZfFabBm3200BwBandwidthParameterEntry_t *pFrame) {

  pFrame->m_nGamma =  (unsigned int)  0;
  pFrame->m_nSigma =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm3200BwBandwidthParameterEntry.hx"



/* Print members in struct */
void
sbZfFabBm3200BwBandwidthParameterEntry_Print(sbZfFabBm3200BwBandwidthParameterEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200BwBandwidthParameterEntry:: gamma=0x%02x"), (unsigned int)  pFromStruct->m_nGamma));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" sigma=0x%06x"), (unsigned int)  pFromStruct->m_nSigma));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
int
sbZfFabBm3200BwBandwidthParameterEntry_SPrint(sbZfFabBm3200BwBandwidthParameterEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwBandwidthParameterEntry:: gamma=0x%02x", (unsigned int)  pFromStruct->m_nGamma);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sigma=0x%06x", (unsigned int)  pFromStruct->m_nSigma);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200BwBandwidthParameterEntry_Validate(sbZfFabBm3200BwBandwidthParameterEntry_t *pZf) {

  if (pZf->m_nGamma > 0xff) return 0;
  if (pZf->m_nSigma > 0xffffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200BwBandwidthParameterEntry_SetField(sbZfFabBm3200BwBandwidthParameterEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ngamma") == 0) {
    s->m_nGamma = value;
  } else if (SB_STRCMP(name, "m_nsigma") == 0) {
    s->m_nSigma = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

