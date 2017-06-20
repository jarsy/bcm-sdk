/*
 * $Id: sbZfKaQmWredCfgTableEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQmWredCfgTableEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQmWredCfgTableEntry_Print(sbZfKaQmWredCfgTableEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQmWredCfgTableEntry:: template=0x%01x"), (unsigned int)  pFromStruct->m_nTemplate));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" gain=0x%01x"), (unsigned int)  pFromStruct->m_nGain));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQmWredCfgTableEntry_SPrint(sbZfKaQmWredCfgTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmWredCfgTableEntry:: template=0x%01x", (unsigned int)  pFromStruct->m_nTemplate);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," gain=0x%01x", (unsigned int)  pFromStruct->m_nGain);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQmWredCfgTableEntry_Validate(sbZfKaQmWredCfgTableEntry_t *pZf) {

  if (pZf->m_nTemplate > 0xf) return 0;
  if (pZf->m_nGain > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQmWredCfgTableEntry_SetField(sbZfKaQmWredCfgTableEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ntemplate") == 0) {
    s->m_nTemplate = value;
  } else if (SB_STRCMP(name, "m_ngain") == 0) {
    s->m_nGain = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
