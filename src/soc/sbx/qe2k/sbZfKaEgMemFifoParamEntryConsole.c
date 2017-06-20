/*
 * $Id: sbZfKaEgMemFifoParamEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include "sbZfKaEgMemShapingEntry.hx"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEgMemFifoParamEntryConsole.hx"



/* Print members in struct */
void
sbZfKaEgMemFifoParamEntry_Print(sbZfKaEgMemFifoParamEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEgMemFifoParamEntry:: threshhi=0x%03x"), (unsigned int)  pFromStruct->m_nThreshHi));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" threshlo=0x%03x"), (unsigned int)  pFromStruct->m_nThreshLo));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" shaper1=0x%02x"), (unsigned int)  pFromStruct->m_nShaper1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEgMemFifoParamEntry:: shaper0=0x%02x"), (unsigned int)  pFromStruct->m_nShaper0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaEgMemFifoParamEntry_SPrint(sbZfKaEgMemFifoParamEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEgMemFifoParamEntry:: threshhi=0x%03x", (unsigned int)  pFromStruct->m_nThreshHi);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," threshlo=0x%03x", (unsigned int)  pFromStruct->m_nThreshLo);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," shaper1=0x%02x", (unsigned int)  pFromStruct->m_nShaper1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEgMemFifoParamEntry:: shaper0=0x%02x", (unsigned int)  pFromStruct->m_nShaper0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEgMemFifoParamEntry_Validate(sbZfKaEgMemFifoParamEntry_t *pZf) {

  if (pZf->m_nThreshHi > 0x3ff) return 0;
  if (pZf->m_nThreshLo > 0x3ff) return 0;
  if (pZf->m_nShaper1 > 0xff) return 0;
  if (pZf->m_nShaper0 > 0xff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEgMemFifoParamEntry_SetField(sbZfKaEgMemFifoParamEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nthreshhi") == 0) {
    s->m_nThreshHi = value;
  } else if (SB_STRCMP(name, "m_nthreshlo") == 0) {
    s->m_nThreshLo = value;
  } else if (SB_STRCMP(name, "m_nshaper1") == 0) {
    s->m_nShaper1 = value;
  } else if (SB_STRCMP(name, "m_nshaper0") == 0) {
    s->m_nShaper0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
