/*
 * $Id: sbZfKaEpIpPrependConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEpIpPrependConsole.hx"



/* Print members in struct */
void
sbZfKaEpIpPrepend_Print(sbZfKaEpIpPrepend_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEpIpPrepend:: data=0x%08x%08x"),   COMPILER_64_HI(pFromStruct->m_nnData), COMPILER_64_LO(pFromStruct->m_nnData)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaEpIpPrepend_SPrint(sbZfKaEpIpPrepend_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpIpPrepend:: data=0x%08x%08x",   COMPILER_64_HI(pFromStruct->m_nnData), COMPILER_64_LO(pFromStruct->m_nnData));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEpIpPrepend_Validate(sbZfKaEpIpPrepend_t *pZf) {

  /* pZf->m_nnData implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEpIpPrepend_SetField(sbZfKaEpIpPrepend_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nndata") == 0) {
    COMPILER_64_SET(s->m_nnData,0,value);
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
