/*
 * $Id: sbZfKaQmQueueArrivalsEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQmQueueArrivalsEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQmQueueArrivalsEntry_Print(sbZfKaQmQueueArrivalsEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQmQueueArrivalsEntry:: arrivals=0x%07x"), (unsigned int)  pFromStruct->m_nArrivals));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQmQueueArrivalsEntry_SPrint(sbZfKaQmQueueArrivalsEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmQueueArrivalsEntry:: arrivals=0x%07x", (unsigned int)  pFromStruct->m_nArrivals);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQmQueueArrivalsEntry_Validate(sbZfKaQmQueueArrivalsEntry_t *pZf) {

  if (pZf->m_nArrivals > 0xfffffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQmQueueArrivalsEntry_SetField(sbZfKaQmQueueArrivalsEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_narrivals") == 0) {
    s->m_nArrivals = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
