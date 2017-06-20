/*
 * $Id: sbZfKaQsShapeRateEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQsShapeRateEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQsShapeRateEntry_Print(sbZfKaQsShapeRateEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsShapeRateEntry:: res=0x%02x"), (unsigned int)  pFromStruct->m_nReserved));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" mode=0x%01x"), (unsigned int)  pFromStruct->m_nMode));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" shaperate=0x%06x"), (unsigned int)  pFromStruct->m_nShapeRate));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQsShapeRateEntry_SPrint(sbZfKaQsShapeRateEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsShapeRateEntry:: res=0x%02x", (unsigned int)  pFromStruct->m_nReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," mode=0x%01x", (unsigned int)  pFromStruct->m_nMode);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," shaperate=0x%06x", (unsigned int)  pFromStruct->m_nShapeRate);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQsShapeRateEntry_Validate(sbZfKaQsShapeRateEntry_t *pZf) {

  if (pZf->m_nReserved > 0xff) return 0;
  if (pZf->m_nMode > 0x1) return 0;
  if (pZf->m_nShapeRate > 0x7fffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQsShapeRateEntry_SetField(sbZfKaQsShapeRateEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved") == 0) {
    s->m_nReserved = value;
  } else if (SB_STRCMP(name, "m_nmode") == 0) {
    s->m_nMode = value;
  } else if (SB_STRCMP(name, "m_nshaperate") == 0) {
    s->m_nShapeRate = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
