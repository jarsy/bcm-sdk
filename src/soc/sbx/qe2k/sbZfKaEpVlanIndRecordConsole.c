/*
 * $Id: sbZfKaEpVlanIndRecordConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEpVlanIndRecordConsole.hx"



/* Print members in struct */
void
sbZfKaEpVlanIndRecord_Print(sbZfKaEpVlanIndRecord_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEpVlanIndRecord:: ptr=0x%04x"), (unsigned int)  pFromStruct->m_nPtr));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" cmap=0x%01x"), (unsigned int)  pFromStruct->m_nCMap));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaEpVlanIndRecord_SPrint(sbZfKaEpVlanIndRecord_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpVlanIndRecord:: ptr=0x%04x", (unsigned int)  pFromStruct->m_nPtr);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," cmap=0x%01x", (unsigned int)  pFromStruct->m_nCMap);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEpVlanIndRecord_Validate(sbZfKaEpVlanIndRecord_t *pZf) {

  if (pZf->m_nPtr > 0x3fff) return 0;
  if (pZf->m_nCMap > 0x3) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEpVlanIndRecord_SetField(sbZfKaEpVlanIndRecord_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nptr") == 0) {
    s->m_nPtr = value;
  } else if (SB_STRCMP(name, "m_ncmap") == 0) {
    s->m_nCMap = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
