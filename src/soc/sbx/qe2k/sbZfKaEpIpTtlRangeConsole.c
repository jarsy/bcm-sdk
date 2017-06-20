/*
 * $Id: sbZfKaEpIpTtlRangeConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEpIpTtlRangeConsole.hx"



/* Print members in struct */
void
sbZfKaEpIpTtlRange_Print(sbZfKaEpIpTtlRange_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEpIpTtlRange:: hi=0x%08x"), (unsigned int)  pFromStruct->m_nHighTtl));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" lo=0x%08x"), (unsigned int)  pFromStruct->m_nLowTtl));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaEpIpTtlRange_SPrint(sbZfKaEpIpTtlRange_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpIpTtlRange:: hi=0x%08x", (unsigned int)  pFromStruct->m_nHighTtl);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," lo=0x%08x", (unsigned int)  pFromStruct->m_nLowTtl);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEpIpTtlRange_Validate(sbZfKaEpIpTtlRange_t *pZf) {

  /* pZf->m_nHighTtl implicitly masked by data type */
  /* pZf->m_nLowTtl implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEpIpTtlRange_SetField(sbZfKaEpIpTtlRange_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nhighttl") == 0) {
    s->m_nHighTtl = value;
  } else if (SB_STRCMP(name, "m_nlowttl") == 0) {
    s->m_nLowTtl = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
