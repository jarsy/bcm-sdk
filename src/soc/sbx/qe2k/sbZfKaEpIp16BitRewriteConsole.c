/*
 * $Id: sbZfKaEpIp16BitRewriteConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEpIp16BitRewriteConsole.hx"



/* Print members in struct */
void
sbZfKaEpIp16BitRewrite_Print(sbZfKaEpIp16BitRewrite_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEpIp16BitRewrite:: data=0x%04x"), (unsigned int)  pFromStruct->m_nData));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaEpIp16BitRewrite_SPrint(sbZfKaEpIp16BitRewrite_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpIp16BitRewrite:: data=0x%04x", (unsigned int)  pFromStruct->m_nData);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEpIp16BitRewrite_Validate(sbZfKaEpIp16BitRewrite_t *pZf) {

  if (pZf->m_nData > 0xffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEpIp16BitRewrite_SetField(sbZfKaEpIp16BitRewrite_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ndata") == 0) {
    s->m_nData = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
