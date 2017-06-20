/*
 * $Id: sbZfKaEpIpPriExpTosRewriteConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEpIpPriExpTosRewriteConsole.hx"



/* Print members in struct */
void
sbZfKaEpIpPriExpTosRewrite_Print(sbZfKaEpIpPriExpTosRewrite_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEpIpPriExpTosRewrite:: tos=0x%02x"), (unsigned int)  pFromStruct->m_nTos));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaEpIpPriExpTosRewrite_SPrint(sbZfKaEpIpPriExpTosRewrite_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpIpPriExpTosRewrite:: tos=0x%02x", (unsigned int)  pFromStruct->m_nTos);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEpIpPriExpTosRewrite_Validate(sbZfKaEpIpPriExpTosRewrite_t *pZf) {

  if (pZf->m_nTos > 0xff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEpIpPriExpTosRewrite_SetField(sbZfKaEpIpPriExpTosRewrite_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ntos") == 0) {
    s->m_nTos = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
