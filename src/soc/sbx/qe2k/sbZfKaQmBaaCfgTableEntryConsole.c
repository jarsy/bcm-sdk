/*
 * $Id: sbZfKaQmBaaCfgTableEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQmBaaCfgTableEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQmBaaCfgTableEntry_Print(sbZfKaQmBaaCfgTableEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQmBaaCfgTableEntry:: gamma=0x%02x"), (unsigned int)  pFromStruct->m_nGamma));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQmBaaCfgTableEntry_SPrint(sbZfKaQmBaaCfgTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmBaaCfgTableEntry:: gamma=0x%02x", (unsigned int)  pFromStruct->m_nGamma);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQmBaaCfgTableEntry_Validate(sbZfKaQmBaaCfgTableEntry_t *pZf) {

  if (pZf->m_nGamma > 0xff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQmBaaCfgTableEntry_SetField(sbZfKaQmBaaCfgTableEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ngamma") == 0) {
    s->m_nGamma = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
