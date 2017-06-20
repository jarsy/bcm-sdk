/*
 * $Id: sbZfKaQsRateTableEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQsRateTableEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQsRateTableEntry_Print(sbZfKaQsRateTableEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsRateTableEntry:: integer=0x%06x"), (unsigned int)  pFromStruct->m_nInteger));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" fraction=0x%02x"), (unsigned int)  pFromStruct->m_nFraction));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQsRateTableEntry_SPrint(sbZfKaQsRateTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsRateTableEntry:: integer=0x%06x", (unsigned int)  pFromStruct->m_nInteger);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," fraction=0x%02x", (unsigned int)  pFromStruct->m_nFraction);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQsRateTableEntry_Validate(sbZfKaQsRateTableEntry_t *pZf) {

  if (pZf->m_nInteger > 0xffffff) return 0;
  if (pZf->m_nFraction > 0xff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQsRateTableEntry_SetField(sbZfKaQsRateTableEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ninteger") == 0) {
    s->m_nInteger = value;
  } else if (SB_STRCMP(name, "m_nfraction") == 0) {
    s->m_nFraction = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
