/*
 * $Id: sbZfKaQmQueueAgeEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQmQueueAgeEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQmQueueAgeEntry_Print(sbZfKaQmQueueAgeEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQmQueueAgeEntry:: age=0x%01x"), (unsigned int)  pFromStruct->m_nAge));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQmQueueAgeEntry_SPrint(sbZfKaQmQueueAgeEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmQueueAgeEntry:: age=0x%01x", (unsigned int)  pFromStruct->m_nAge);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQmQueueAgeEntry_Validate(sbZfKaQmQueueAgeEntry_t *pZf) {

  if (pZf->m_nAge > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQmQueueAgeEntry_SetField(sbZfKaQmQueueAgeEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nage") == 0) {
    s->m_nAge = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
