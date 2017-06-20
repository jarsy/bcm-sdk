/*
 * $Id: sbZfKaQmPortBwCfgTableEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQmPortBwCfgTableEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQmPortBwCfgTableEntry_Print(sbZfKaQmPortBwCfgTableEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQmPortBwCfgTableEntry:: spqueues=0x%02x"), (unsigned int)  pFromStruct->m_nSpQueues));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" queues=0x%02x"), (unsigned int)  pFromStruct->m_nQueues));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" basequeue=0x%04x"), (unsigned int)  pFromStruct->m_nBaseQueue));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQmPortBwCfgTableEntry:: linerate=0x%06x"), (unsigned int)  pFromStruct->m_nLineRate));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQmPortBwCfgTableEntry_SPrint(sbZfKaQmPortBwCfgTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmPortBwCfgTableEntry:: spqueues=0x%02x", (unsigned int)  pFromStruct->m_nSpQueues);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," queues=0x%02x", (unsigned int)  pFromStruct->m_nQueues);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," basequeue=0x%04x", (unsigned int)  pFromStruct->m_nBaseQueue);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmPortBwCfgTableEntry:: linerate=0x%06x", (unsigned int)  pFromStruct->m_nLineRate);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQmPortBwCfgTableEntry_Validate(sbZfKaQmPortBwCfgTableEntry_t *pZf) {

  if (pZf->m_nSpQueues > 0x1f) return 0;
  if (pZf->m_nQueues > 0x1f) return 0;
  if (pZf->m_nBaseQueue > 0x3fff) return 0;
  if (pZf->m_nLineRate > 0x3fffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQmPortBwCfgTableEntry_SetField(sbZfKaQmPortBwCfgTableEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nspqueues") == 0) {
    s->m_nSpQueues = value;
  } else if (SB_STRCMP(name, "m_nqueues") == 0) {
    s->m_nQueues = value;
  } else if (SB_STRCMP(name, "m_nbasequeue") == 0) {
    s->m_nBaseQueue = value;
  } else if (SB_STRCMP(name, "m_nlinerate") == 0) {
    s->m_nLineRate = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
