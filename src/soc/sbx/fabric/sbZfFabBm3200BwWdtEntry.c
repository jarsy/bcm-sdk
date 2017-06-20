/*
 * $Id: sbZfFabBm3200BwWdtEntry.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <shared/bsl.h>

#include "sbTypes.h"
#include "sbZfFabBm3200BwWdtEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm3200BwWdtEntry_Pack(sbZfFabBm3200BwWdtEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_WDT_ENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nTemplate */
  (pToData)[2] |= ((pFrom)->m_nTemplate) & 0xFF;

  /* Pack Member: m_nSpare */
  (pToData)[3] |= ((pFrom)->m_nSpare & 0x0f) <<4;

  /* Pack Member: m_nGain */
  (pToData)[3] |= ((pFrom)->m_nGain & 0x0f);
#else
  int i;
  int size = SB_ZF_FAB_BM3200_WDT_ENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nTemplate */
  (pToData)[1] |= ((pFrom)->m_nTemplate) & 0xFF;

  /* Pack Member: m_nSpare */
  (pToData)[0] |= ((pFrom)->m_nSpare & 0x0f) <<4;

  /* Pack Member: m_nGain */
  (pToData)[0] |= ((pFrom)->m_nGain & 0x0f);
#endif

  return SB_ZF_FAB_BM3200_WDT_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200BwWdtEntry_Unpack(sbZfFabBm3200BwWdtEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nTemplate */
  (pToStruct)->m_nTemplate =  (uint32)  (pFromData)[2] ;

  /* Unpack Member: m_nSpare */
  (pToStruct)->m_nSpare =  (uint32)  ((pFromData)[3] >> 4) & 0x0f;

  /* Unpack Member: m_nGain */
  (pToStruct)->m_nGain =  (uint32)  ((pFromData)[3] ) & 0x0f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nTemplate */
  (pToStruct)->m_nTemplate =  (uint32)  (pFromData)[1] ;

  /* Unpack Member: m_nSpare */
  (pToStruct)->m_nSpare =  (uint32)  ((pFromData)[0] >> 4) & 0x0f;

  /* Unpack Member: m_nGain */
  (pToStruct)->m_nGain =  (uint32)  ((pFromData)[0] ) & 0x0f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200BwWdtEntry_InitInstance(sbZfFabBm3200BwWdtEntry_t *pFrame) {

  pFrame->m_nTemplate =  (unsigned int)  0;
  pFrame->m_nSpare =  (unsigned int)  0;
  pFrame->m_nGain =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabBm3200BwWdtEntry.c,v 1.4 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm3200BwWdtEntry.hx"



/* Print members in struct */
void
sbZfFabBm3200BwWdtEntry_Print(sbZfFabBm3200BwWdtEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200BwWdtEntry:: template=0x%02x"), (unsigned int)  pFromStruct->m_nTemplate));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" spare=0x%01x"), (unsigned int)  pFromStruct->m_nSpare));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" gain=0x%01x"), (unsigned int)  pFromStruct->m_nGain));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
int
sbZfFabBm3200BwWdtEntry_SPrint(sbZfFabBm3200BwWdtEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwWdtEntry:: template=0x%02x", (unsigned int)  pFromStruct->m_nTemplate);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spare=0x%01x", (unsigned int)  pFromStruct->m_nSpare);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," gain=0x%01x", (unsigned int)  pFromStruct->m_nGain);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200BwWdtEntry_Validate(sbZfFabBm3200BwWdtEntry_t *pZf) {

  if (pZf->m_nTemplate > 0xff) return 0;
  if (pZf->m_nSpare > 0xf) return 0;
  if (pZf->m_nGain > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200BwWdtEntry_SetField(sbZfFabBm3200BwWdtEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ntemplate") == 0) {
    s->m_nTemplate = value;
  } else if (SB_STRCMP(name, "m_nspare") == 0) {
    s->m_nSpare = value;
  } else if (SB_STRCMP(name, "m_ngain") == 0) {
    s->m_nGain = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

