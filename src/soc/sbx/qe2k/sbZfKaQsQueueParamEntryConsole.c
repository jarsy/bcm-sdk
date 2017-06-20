/*
 * $Id: sbZfKaQsQueueParamEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQsQueueParamEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQsQueueParamEntry_Print(sbZfKaQsQueueParamEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsQueueParamEntry:: res=0x%06x"), (unsigned int)  pFromStruct->m_nReserved));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" local=0x%01x"), (unsigned int)  pFromStruct->m_nLocal));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" maxholdts=0x%01x"), (unsigned int)  pFromStruct->m_nMaxHoldTs));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" qtype=0x%01x"), (unsigned int)  pFromStruct->m_nQType));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQsQueueParamEntry_SPrint(sbZfKaQsQueueParamEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsQueueParamEntry:: res=0x%06x", (unsigned int)  pFromStruct->m_nReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," local=0x%01x", (unsigned int)  pFromStruct->m_nLocal);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," maxholdts=0x%01x", (unsigned int)  pFromStruct->m_nMaxHoldTs);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," qtype=0x%01x", (unsigned int)  pFromStruct->m_nQType);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQsQueueParamEntry_Validate(sbZfKaQsQueueParamEntry_t *pZf) {

  if (pZf->m_nReserved > 0xffffff) return 0;
  if (pZf->m_nLocal > 0x1) return 0;
  if (pZf->m_nMaxHoldTs > 0x7) return 0;
  if (pZf->m_nQType > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQsQueueParamEntry_SetField(sbZfKaQsQueueParamEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved") == 0) {
    s->m_nReserved = value;
  } else if (SB_STRCMP(name, "m_nlocal") == 0) {
    s->m_nLocal = value;
  } else if (SB_STRCMP(name, "m_nmaxholdts") == 0) {
    s->m_nMaxHoldTs = value;
  } else if (SB_STRCMP(name, "m_nqtype") == 0) {
    s->m_nQType = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
