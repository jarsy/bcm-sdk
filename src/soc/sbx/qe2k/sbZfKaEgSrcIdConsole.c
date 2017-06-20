/*
 * $Id: sbZfKaEgSrcIdConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEgSrcIdConsole.hx"



/* Print members in struct */
void
sbZfKaEgSrcId_Print(sbZfKaEgSrcId_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEgSrcId:: src_id=0x%03x"), (unsigned int)  pFromStruct->m_nSrcId));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaEgSrcId_SPrint(sbZfKaEgSrcId_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEgSrcId:: src_id=0x%03x", (unsigned int)  pFromStruct->m_nSrcId);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEgSrcId_Validate(sbZfKaEgSrcId_t *pZf) {

  if (pZf->m_nSrcId > 0xfff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEgSrcId_SetField(sbZfKaEgSrcId_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nsrcid") == 0) {
    s->m_nSrcId = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
