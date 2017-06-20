/*
 * $Id: sbZfKaQmQueueByteAdjEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQmQueueByteAdjEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQmQueueByteAdjEntry_Print(sbZfKaQmQueueByteAdjEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQmQueueByteAdjEntry:: sign=0x%01x"), (unsigned int)  pFromStruct->m_nSign));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" bytes=0x%02x"), (unsigned int)  pFromStruct->m_nBytes));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQmQueueByteAdjEntry_SPrint(sbZfKaQmQueueByteAdjEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmQueueByteAdjEntry:: sign=0x%01x", (unsigned int)  pFromStruct->m_nSign);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytes=0x%02x", (unsigned int)  pFromStruct->m_nBytes);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQmQueueByteAdjEntry_Validate(sbZfKaQmQueueByteAdjEntry_t *pZf) {

  if (pZf->m_nSign > 0x1) return 0;
  if (pZf->m_nBytes > 0x3f) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQmQueueByteAdjEntry_SetField(sbZfKaQmQueueByteAdjEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nsign") == 0) {
    s->m_nSign = value;
  } else if (SB_STRCMP(name, "m_nbytes") == 0) {
    s->m_nBytes = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
