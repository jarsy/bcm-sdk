/*
 * $Id: sbZfFabBm3200BwWctDpEntry.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <shared/bsl.h>

#include "sbTypes.h"
#include "sbZfFabBm3200BwWctDpEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm3200BwWctDpEntry_Pack(sbZfFabBm3200BwWctDpEntry_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_WCT_DPENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nEcn */
  (pToData)[5] |= ((pFrom)->m_nEcn) & 0xFF;
  (pToData)[4] |= ((pFrom)->m_nEcn >> 8) &0xFF;

  /* Pack Member: m_nScale */
  (pToData)[6] |= ((pFrom)->m_nScale & 0x0f) <<4;

  /* Pack Member: m_nSlope */
  (pToData)[7] |= ((pFrom)->m_nSlope) & 0xFF;
  (pToData)[6] |= ((pFrom)->m_nSlope >> 8) & 0x0f;

  /* Pack Member: m_nMin */
  (pToData)[1] |= ((pFrom)->m_nMin) & 0xFF;
  (pToData)[0] |= ((pFrom)->m_nMin >> 8) &0xFF;

  /* Pack Member: m_nMax */
  (pToData)[3] |= ((pFrom)->m_nMax) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nMax >> 8) &0xFF;
#else
  int i;
  int size = SB_ZF_FAB_BM3200_WCT_DPENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nEcn */
  (pToData)[6] |= ((pFrom)->m_nEcn) & 0xFF;
  (pToData)[7] |= ((pFrom)->m_nEcn >> 8) &0xFF;

  /* Pack Member: m_nScale */
  (pToData)[5] |= ((pFrom)->m_nScale & 0x0f) <<4;

  /* Pack Member: m_nSlope */
  (pToData)[4] |= ((pFrom)->m_nSlope) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_nSlope >> 8) & 0x0f;

  /* Pack Member: m_nMin */
  (pToData)[2] |= ((pFrom)->m_nMin) & 0xFF;
  (pToData)[3] |= ((pFrom)->m_nMin >> 8) &0xFF;

  /* Pack Member: m_nMax */
  (pToData)[0] |= ((pFrom)->m_nMax) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nMax >> 8) &0xFF;
#endif

  return SB_ZF_FAB_BM3200_WCT_DPENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200BwWctDpEntry_Unpack(sbZfFabBm3200BwWctDpEntry_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nEcn */
  (pToStruct)->m_nEcn =  (uint32)  (pFromData)[5] ;
  (pToStruct)->m_nEcn |=  (uint32)  (pFromData)[4] << 8;

  /* Unpack Member: m_nScale */
  (pToStruct)->m_nScale =  (uint32)  ((pFromData)[6] >> 4) & 0x0f;

  /* Unpack Member: m_nSlope */
  (pToStruct)->m_nSlope =  (uint32)  (pFromData)[7] ;
  (pToStruct)->m_nSlope |=  (uint32)  ((pFromData)[6] & 0x0f) << 8;

  /* Unpack Member: m_nMin */
  (pToStruct)->m_nMin =  (uint32)  (pFromData)[1] ;
  (pToStruct)->m_nMin |=  (uint32)  (pFromData)[0] << 8;

  /* Unpack Member: m_nMax */
  (pToStruct)->m_nMax =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nMax |=  (uint32)  (pFromData)[2] << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nEcn */
  (pToStruct)->m_nEcn =  (uint32)  (pFromData)[6] ;
  (pToStruct)->m_nEcn |=  (uint32)  (pFromData)[7] << 8;

  /* Unpack Member: m_nScale */
  (pToStruct)->m_nScale =  (uint32)  ((pFromData)[5] >> 4) & 0x0f;

  /* Unpack Member: m_nSlope */
  (pToStruct)->m_nSlope =  (uint32)  (pFromData)[4] ;
  (pToStruct)->m_nSlope |=  (uint32)  ((pFromData)[5] & 0x0f) << 8;

  /* Unpack Member: m_nMin */
  (pToStruct)->m_nMin =  (uint32)  (pFromData)[2] ;
  (pToStruct)->m_nMin |=  (uint32)  (pFromData)[3] << 8;

  /* Unpack Member: m_nMax */
  (pToStruct)->m_nMax =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nMax |=  (uint32)  (pFromData)[1] << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200BwWctDpEntry_InitInstance(sbZfFabBm3200BwWctDpEntry_t *pFrame) {

  pFrame->m_nEcn =  (unsigned int)  0;
  pFrame->m_nScale =  (unsigned int)  0;
  pFrame->m_nSlope =  (unsigned int)  0;
  pFrame->m_nMin =  (unsigned int)  0;
  pFrame->m_nMax =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabBm3200BwWctDpEntry.c,v 1.4 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm3200BwWctDpEntry.hx"



/* Print members in struct */
void
sbZfFabBm3200BwWctDpEntry_Print(sbZfFabBm3200BwWctDpEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200BwWctDpEntry:: ecn=0x%04x"), (unsigned int)  pFromStruct->m_nEcn));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" scale=0x%01x"), (unsigned int)  pFromStruct->m_nScale));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" slope=0x%03x"), (unsigned int)  pFromStruct->m_nSlope));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" min=0x%04x"), (unsigned int)  pFromStruct->m_nMin));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200BwWctDpEntry:: max=0x%04x"), (unsigned int)  pFromStruct->m_nMax));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
int
sbZfFabBm3200BwWctDpEntry_SPrint(sbZfFabBm3200BwWctDpEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwWctDpEntry:: ecn=0x%04x", (unsigned int)  pFromStruct->m_nEcn);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," scale=0x%01x", (unsigned int)  pFromStruct->m_nScale);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," slope=0x%03x", (unsigned int)  pFromStruct->m_nSlope);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," min=0x%04x", (unsigned int)  pFromStruct->m_nMin);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwWctDpEntry:: max=0x%04x", (unsigned int)  pFromStruct->m_nMax);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200BwWctDpEntry_Validate(sbZfFabBm3200BwWctDpEntry_t *pZf) {

  if (pZf->m_nEcn > 0xffff) return 0;
  if (pZf->m_nScale > 0xf) return 0;
  if (pZf->m_nSlope > 0xfff) return 0;
  if (pZf->m_nMin > 0xffff) return 0;
  if (pZf->m_nMax > 0xffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200BwWctDpEntry_SetField(sbZfFabBm3200BwWctDpEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_necn") == 0) {
    s->m_nEcn = value;
  } else if (SB_STRCMP(name, "m_nscale") == 0) {
    s->m_nScale = value;
  } else if (SB_STRCMP(name, "m_nslope") == 0) {
    s->m_nSlope = value;
  } else if (SB_STRCMP(name, "m_nmin") == 0) {
    s->m_nMin = value;
  } else if (SB_STRCMP(name, "m_nmax") == 0) {
    s->m_nMax = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

