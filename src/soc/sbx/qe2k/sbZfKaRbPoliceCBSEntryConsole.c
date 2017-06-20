/*
 * $Id: sbZfKaRbPoliceCBSEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaRbPoliceCBSEntryConsole.hx"



/* Print members in struct */
void
sbZfKaRbPoliceCBSEntry_Print(sbZfKaRbPoliceCBSEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbPoliceCBSEntry:: cbs=0x%06x"), (unsigned int)  pFromStruct->m_nCBS));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaRbPoliceCBSEntry_SPrint(sbZfKaRbPoliceCBSEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbPoliceCBSEntry:: cbs=0x%06x", (unsigned int)  pFromStruct->m_nCBS);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaRbPoliceCBSEntry_Validate(sbZfKaRbPoliceCBSEntry_t *pZf) {

  if (pZf->m_nCBS > 0xffffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaRbPoliceCBSEntry_SetField(sbZfKaRbPoliceCBSEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ncbs") == 0) {
    s->m_nCBS = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
