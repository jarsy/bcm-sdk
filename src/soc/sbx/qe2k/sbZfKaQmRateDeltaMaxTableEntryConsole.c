/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQmRateDeltaMaxTableEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQmRateDeltaMaxTableEntry_Print(sbZfKaQmRateDeltaMaxTableEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQmRateDeltaMaxTableEntry:: rate_delta_max=0x%07x"), (unsigned int)  pFromStruct->m_nRateDeltaMax));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQmRateDeltaMaxTableEntry_SPrint(sbZfKaQmRateDeltaMaxTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmRateDeltaMaxTableEntry:: rate_delta_max=0x%07x", (unsigned int)  pFromStruct->m_nRateDeltaMax);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQmRateDeltaMaxTableEntry_Validate(sbZfKaQmRateDeltaMaxTableEntry_t *pZf) {

  if (pZf->m_nRateDeltaMax > 0x7ffffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQmRateDeltaMaxTableEntry_SetField(sbZfKaQmRateDeltaMaxTableEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nratedeltamax") == 0) {
    s->m_nRateDeltaMax = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
