/*
 * $Id: sbZfKaQmWredAvQlenTableEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQmWredAvQlenTableEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQmWredAvQlenTableEntry_Print(sbZfKaQmWredAvQlenTableEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQmWredAvQlenTableEntry:: queueavg=0x%07x"), (unsigned int)  pFromStruct->m_nQueueAvg));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQmWredAvQlenTableEntry_SPrint(sbZfKaQmWredAvQlenTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmWredAvQlenTableEntry:: queueavg=0x%07x", (unsigned int)  pFromStruct->m_nQueueAvg);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQmWredAvQlenTableEntry_Validate(sbZfKaQmWredAvQlenTableEntry_t *pZf) {

  if (pZf->m_nQueueAvg > 0x1ffffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQmWredAvQlenTableEntry_SetField(sbZfKaQmWredAvQlenTableEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nqueueavg") == 0) {
    s->m_nQueueAvg = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
