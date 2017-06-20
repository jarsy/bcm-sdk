/*
 * $Id: sbZfFabBm3200BwQlopEntry.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <shared/bsl.h>

#include "sbTypes.h"
#include "sbZfFabBm3200BwQlopEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm3200BwQlopEntry_Pack(sbZfFabBm3200BwQlopEntry_t *pFrom,
                              uint8 *pToData,
                              uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_QLOP_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nAlpha */
  (pToData)[1] |= ((pFrom)->m_nAlpha & 0x01) <<7;
  (pToData)[0] |= ((pFrom)->m_nAlpha >> 1) & 0x03;

  /* Pack Member: m_nBeta */
  (pToData)[1] |= ((pFrom)->m_nBeta & 0x07) <<4;

  /* Pack Member: m_nEpsilon */
  (pToData)[2] |= ((pFrom)->m_nEpsilon & 0x3f) <<2;
  (pToData)[1] |= ((pFrom)->m_nEpsilon >> 6) & 0x0f;

  /* Pack Member: m_nRateDeltaMax */
  (pToData)[3] |= ((pFrom)->m_nRateDeltaMax) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nRateDeltaMax >> 8) & 0x03;
#else
  int i;
  int size = SB_ZF_FAB_BM3200_QLOP_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nAlpha */
  (pToData)[2] |= ((pFrom)->m_nAlpha & 0x01) <<7;
  (pToData)[3] |= ((pFrom)->m_nAlpha >> 1) & 0x03;

  /* Pack Member: m_nBeta */
  (pToData)[2] |= ((pFrom)->m_nBeta & 0x07) <<4;

  /* Pack Member: m_nEpsilon */
  (pToData)[1] |= ((pFrom)->m_nEpsilon & 0x3f) <<2;
  (pToData)[2] |= ((pFrom)->m_nEpsilon >> 6) & 0x0f;

  /* Pack Member: m_nRateDeltaMax */
  (pToData)[0] |= ((pFrom)->m_nRateDeltaMax) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nRateDeltaMax >> 8) & 0x03;
#endif

  return SB_ZF_FAB_BM3200_QLOP_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200BwQlopEntry_Unpack(sbZfFabBm3200BwQlopEntry_t *pToStruct,
                                uint8 *pFromData,
                                uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nAlpha */
  (pToStruct)->m_nAlpha =  (uint32)  ((pFromData)[1] >> 7) & 0x01;
  (pToStruct)->m_nAlpha |=  (uint32)  ((pFromData)[0] & 0x03) << 1;

  /* Unpack Member: m_nBeta */
  (pToStruct)->m_nBeta =  (uint32)  ((pFromData)[1] >> 4) & 0x07;

  /* Unpack Member: m_nEpsilon */
  (pToStruct)->m_nEpsilon =  (uint32)  ((pFromData)[2] >> 2) & 0x3f;
  (pToStruct)->m_nEpsilon |=  (uint32)  ((pFromData)[1] & 0x0f) << 6;

  /* Unpack Member: m_nRateDeltaMax */
  (pToStruct)->m_nRateDeltaMax =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nRateDeltaMax |=  (uint32)  ((pFromData)[2] & 0x03) << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nAlpha */
  (pToStruct)->m_nAlpha =  (uint32)  ((pFromData)[2] >> 7) & 0x01;
  (pToStruct)->m_nAlpha |=  (uint32)  ((pFromData)[3] & 0x03) << 1;

  /* Unpack Member: m_nBeta */
  (pToStruct)->m_nBeta =  (uint32)  ((pFromData)[2] >> 4) & 0x07;

  /* Unpack Member: m_nEpsilon */
  (pToStruct)->m_nEpsilon =  (uint32)  ((pFromData)[1] >> 2) & 0x3f;
  (pToStruct)->m_nEpsilon |=  (uint32)  ((pFromData)[2] & 0x0f) << 6;

  /* Unpack Member: m_nRateDeltaMax */
  (pToStruct)->m_nRateDeltaMax =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nRateDeltaMax |=  (uint32)  ((pFromData)[1] & 0x03) << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200BwQlopEntry_InitInstance(sbZfFabBm3200BwQlopEntry_t *pFrame) {

  pFrame->m_nAlpha =  (unsigned int)  0;
  pFrame->m_nBeta =  (unsigned int)  0;
  pFrame->m_nEpsilon =  (unsigned int)  0;
  pFrame->m_nRateDeltaMax =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabBm3200BwQlopEntry.c,v 1.4 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm3200BwQlopEntry.hx"



/* Print members in struct */
void
sbZfFabBm3200BwQlopEntry_Print(sbZfFabBm3200BwQlopEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200BwQlopEntry:: alpha=0x%01x"), (unsigned int)  pFromStruct->m_nAlpha));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" beta=0x%01x"), (unsigned int)  pFromStruct->m_nBeta));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" epsilon=0x%03x"), (unsigned int)  pFromStruct->m_nEpsilon));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" ratedeltamax=0x%03x"), (unsigned int)  pFromStruct->m_nRateDeltaMax));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
int
sbZfFabBm3200BwQlopEntry_SPrint(sbZfFabBm3200BwQlopEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwQlopEntry:: alpha=0x%01x", (unsigned int)  pFromStruct->m_nAlpha);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," beta=0x%01x", (unsigned int)  pFromStruct->m_nBeta);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," epsilon=0x%03x", (unsigned int)  pFromStruct->m_nEpsilon);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ratedeltamax=0x%03x", (unsigned int)  pFromStruct->m_nRateDeltaMax);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200BwQlopEntry_Validate(sbZfFabBm3200BwQlopEntry_t *pZf) {

  if (pZf->m_nAlpha > 0x7) return 0;
  if (pZf->m_nBeta > 0x7) return 0;
  if (pZf->m_nEpsilon > 0x3ff) return 0;
  if (pZf->m_nRateDeltaMax > 0x3ff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200BwQlopEntry_SetField(sbZfFabBm3200BwQlopEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nalpha") == 0) {
    s->m_nAlpha = value;
  } else if (SB_STRCMP(name, "m_nbeta") == 0) {
    s->m_nBeta = value;
  } else if (SB_STRCMP(name, "m_nepsilon") == 0) {
    s->m_nEpsilon = value;
  } else if (SB_STRCMP(name, "m_nratedeltamax") == 0) {
    s->m_nRateDeltaMax = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

