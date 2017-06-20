/*
 * $Id: sbZfFabBm3200BwDstEntry.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <shared/bsl.h>

#include "sbTypes.h"
#include "sbZfFabBm3200BwDstEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm3200BwDstEntry_Pack(sbZfFabBm3200BwDstEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_DST_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nDelta */
  (pToData)[1] |= ((pFrom)->m_nDelta & 0x03) <<6;
  (pToData)[0] |= ((pFrom)->m_nDelta >> 2) &0xFF;

  /* Pack Member: m_nDemand */
  (pToData)[3] |= ((pFrom)->m_nDemand) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nDemand >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nDemand >> 16) & 0x3f;
#else
  int i;
  int size = SB_ZF_FAB_BM3200_DST_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nDelta */
  (pToData)[2] |= ((pFrom)->m_nDelta & 0x03) <<6;
  (pToData)[3] |= ((pFrom)->m_nDelta >> 2) &0xFF;

  /* Pack Member: m_nDemand */
  (pToData)[0] |= ((pFrom)->m_nDemand) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nDemand >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nDemand >> 16) & 0x3f;
#endif

  return SB_ZF_FAB_BM3200_DST_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200BwDstEntry_Unpack(sbZfFabBm3200BwDstEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nDelta */
  (pToStruct)->m_nDelta =  (uint32)  ((pFromData)[1] >> 6) & 0x03;
  (pToStruct)->m_nDelta |=  (uint32)  (pFromData)[0] << 2;

  /* Unpack Member: m_nDemand */
  (pToStruct)->m_nDemand =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nDemand |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_nDemand |=  (uint32)  ((pFromData)[1] & 0x3f) << 16;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nDelta */
  (pToStruct)->m_nDelta =  (uint32)  ((pFromData)[2] >> 6) & 0x03;
  (pToStruct)->m_nDelta |=  (uint32)  (pFromData)[3] << 2;

  /* Unpack Member: m_nDemand */
  (pToStruct)->m_nDemand =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nDemand |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_nDemand |=  (uint32)  ((pFromData)[2] & 0x3f) << 16;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200BwDstEntry_InitInstance(sbZfFabBm3200BwDstEntry_t *pFrame) {

  pFrame->m_nDelta =  (unsigned int)  0;
  pFrame->m_nDemand =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabBm3200BwDstEntry.c,v 1.4 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm3200BwDstEntry.hx"



/* Print members in struct */
void
sbZfFabBm3200BwDstEntry_Print(sbZfFabBm3200BwDstEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200BwDstEntry:: delta=0x%03x"), (unsigned int)  pFromStruct->m_nDelta));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" demand=0x%06x"), (unsigned int)  pFromStruct->m_nDemand));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
int
sbZfFabBm3200BwDstEntry_SPrint(sbZfFabBm3200BwDstEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwDstEntry:: delta=0x%03x", (unsigned int)  pFromStruct->m_nDelta);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," demand=0x%06x", (unsigned int)  pFromStruct->m_nDemand);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200BwDstEntry_Validate(sbZfFabBm3200BwDstEntry_t *pZf) {

  if (pZf->m_nDelta > 0x3ff) return 0;
  if (pZf->m_nDemand > 0x3fffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200BwDstEntry_SetField(sbZfFabBm3200BwDstEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ndelta") == 0) {
    s->m_nDelta = value;
  } else if (SB_STRCMP(name, "m_ndemand") == 0) {
    s->m_nDemand = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

