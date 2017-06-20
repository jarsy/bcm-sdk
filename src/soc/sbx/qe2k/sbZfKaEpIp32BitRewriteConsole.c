/*
 * $Id: sbZfKaEpIp32BitRewriteConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEpIp32BitRewriteConsole.hx"



/* Print members in struct */
void
sbZfKaEpIp32BitRewrite_Print(sbZfKaEpIp32BitRewrite_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEpIp32BitRewrite:: data=0x%08x"), (unsigned int)  pFromStruct->m_nData));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaEpIp32BitRewrite_SPrint(sbZfKaEpIp32BitRewrite_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpIp32BitRewrite:: data=0x%08x", (unsigned int)  pFromStruct->m_nData);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEpIp32BitRewrite_Validate(sbZfKaEpIp32BitRewrite_t *pZf) {

  /* pZf->m_nData implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEpIp32BitRewrite_SetField(sbZfKaEpIp32BitRewrite_t *s, char* name, int value) {

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
