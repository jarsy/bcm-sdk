/*
 * $Id: sbZfFabWredTableEntry.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <shared/bsl.h>

#include "sbTypes.h"
#include "sbZfFabWredParameters.hx"
#include "sbZfFabWredTableEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabWredTableEntry_Pack(sbZfFabWredTableEntry_t *pFrom,
                           uint8 *pToData,
                           uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_WRED_TABLE_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nDp0 */

  sbZfFabWredParameters_Pack(&((pFrom)->m_nDp0), (pToData)+16, nMaxToDataIndex-16);

  /* Pack Member: m_nDp1 */

  sbZfFabWredParameters_Pack(&((pFrom)->m_nDp1), (pToData)+8, nMaxToDataIndex-8);

  /* Pack Member: m_nDp2 */

  sbZfFabWredParameters_Pack(&((pFrom)->m_nDp2), (pToData)+0, nMaxToDataIndex-0);
#else
  int i;
  int size = SB_ZF_WRED_TABLE_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nDp0 */

  sbZfFabWredParameters_Pack(&((pFrom)->m_nDp0), (pToData)+16, nMaxToDataIndex-16);

  /* Pack Member: m_nDp1 */

  sbZfFabWredParameters_Pack(&((pFrom)->m_nDp1), (pToData)+8, nMaxToDataIndex-8);

  /* Pack Member: m_nDp2 */

  sbZfFabWredParameters_Pack(&((pFrom)->m_nDp2), (pToData)+0, nMaxToDataIndex-0);
#endif

  return SB_ZF_WRED_TABLE_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabWredTableEntry_Unpack(sbZfFabWredTableEntry_t *pToStruct,
                             uint8 *pFromData,
                             uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nDp0 */

  sbZfFabWredParameters_Unpack(&((pToStruct)->m_nDp0), (pFromData)+16, nMaxToDataIndex-16);

  /* Unpack Member: m_nDp1 */

  sbZfFabWredParameters_Unpack(&((pToStruct)->m_nDp1), (pFromData)+8, nMaxToDataIndex-8);

  /* Unpack Member: m_nDp2 */

  sbZfFabWredParameters_Unpack(&((pToStruct)->m_nDp2), (pFromData)+0, nMaxToDataIndex-0);
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nDp0 */

  sbZfFabWredParameters_Unpack(&((pToStruct)->m_nDp0), (pFromData)+16, nMaxToDataIndex-16);

  /* Unpack Member: m_nDp1 */

  sbZfFabWredParameters_Unpack(&((pToStruct)->m_nDp1), (pFromData)+8, nMaxToDataIndex-8);

  /* Unpack Member: m_nDp2 */

  sbZfFabWredParameters_Unpack(&((pToStruct)->m_nDp2), (pFromData)+0, nMaxToDataIndex-0);
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabWredTableEntry_InitInstance(sbZfFabWredTableEntry_t *pFrame) {

  sbZfFabWredParameters_InitInstance(&(pFrame->m_nDp0));
  sbZfFabWredParameters_InitInstance(&(pFrame->m_nDp1));
  sbZfFabWredParameters_InitInstance(&(pFrame->m_nDp2));

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabWredTableEntry.c,v 1.3 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include "sbZfFabWredParameters.hx"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabWredTableEntry.hx"



/* Print members in struct */
void
sbZfFabWredTableEntry_Print(sbZfFabWredTableEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabWredTableEntry:: dp0 below:\n")));
  sbZfFabWredParameters_Print(&(pFromStruct->m_nDp0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabWredTableEntry:: dp1 below:\n")));
  sbZfFabWredParameters_Print(&(pFromStruct->m_nDp1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabWredTableEntry:: dp2 below:\n")));
  sbZfFabWredParameters_Print(&(pFromStruct->m_nDp2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
int
sbZfFabWredTableEntry_SPrint(sbZfFabWredTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += sbZfFabWredParameters_SPrint(&(pFromStruct->m_nDp0), &pcToString[WrCnt], lStrSize-WrCnt);
  WrCnt += sbZfFabWredParameters_SPrint(&(pFromStruct->m_nDp1), &pcToString[WrCnt], lStrSize-WrCnt);
  WrCnt += sbZfFabWredParameters_SPrint(&(pFromStruct->m_nDp2), &pcToString[WrCnt], lStrSize-WrCnt);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabWredTableEntry_Validate(sbZfFabWredTableEntry_t *pZf) {

  if (!sbZfFabWredParameters_Validate(&pZf->m_nDp0)) return 0;
  if (!sbZfFabWredParameters_Validate(&pZf->m_nDp1)) return 0;
  if (!sbZfFabWredParameters_Validate(&pZf->m_nDp2)) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabWredTableEntry_SetField(sbZfFabWredTableEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRSTR(name, "m_ndp0.") == name) {
    sbZfFabWredParameters_SetField(&(s->m_nDp0), name+7, value);
  } else if (SB_STRSTR(name, "m_ndp1.") == name) {
    sbZfFabWredParameters_SetField(&(s->m_nDp1), name+7, value);
  } else if (SB_STRSTR(name, "m_ndp2.") == name) {
    sbZfFabWredParameters_SetField(&(s->m_nDp2), name+7, value);
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

