/*
 * $Id: sbZfKaQsE2QEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQsE2QEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQsE2QEntry_Print(sbZfKaQsE2QEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsE2QEntry:: res=0x%05x"), (unsigned int)  pFromStruct->m_nReserved));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable=0x%01x"), (unsigned int)  pFromStruct->m_nEnable));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" queuenum=0x%04x"), (unsigned int)  pFromStruct->m_nQueueNum));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQsE2QEntry_SPrint(sbZfKaQsE2QEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsE2QEntry:: res=0x%05x", (unsigned int)  pFromStruct->m_nReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable=0x%01x", (unsigned int)  pFromStruct->m_nEnable);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," queuenum=0x%04x", (unsigned int)  pFromStruct->m_nQueueNum);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQsE2QEntry_Validate(sbZfKaQsE2QEntry_t *pZf) {

  if (pZf->m_nReserved > 0x1ffff) return 0;
  if (pZf->m_nEnable > 0x1) return 0;
  if (pZf->m_nQueueNum > 0x3fff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQsE2QEntry_SetField(sbZfKaQsE2QEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved") == 0) {
    s->m_nReserved = value;
  } else if (SB_STRCMP(name, "m_nenable") == 0) {
    s->m_nEnable = value;
  } else if (SB_STRCMP(name, "m_nqueuenum") == 0) {
    s->m_nQueueNum = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
