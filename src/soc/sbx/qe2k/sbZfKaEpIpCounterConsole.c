/*
 * $Id: sbZfKaEpIpCounterConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEpIpCounterConsole.hx"



/* Print members in struct */
void
sbZfKaEpIpCounter_Print(sbZfKaEpIpCounter_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEpIpCounter:: pktcnt=0x%08x"), (unsigned int)  pFromStruct->m_nPktCnt));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" bytecnt=0x%01x%08x"), COMPILER_64_HI(pFromStruct->m_nnByteCnt), COMPILER_64_LO(pFromStruct->m_nnByteCnt)));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaEpIpCounter_SPrint(sbZfKaEpIpCounter_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpIpCounter:: pktcnt=0x%08x", (unsigned int)  pFromStruct->m_nPktCnt);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt=0x%01x%08x",  COMPILER_64_HI(pFromStruct->m_nnByteCnt), COMPILER_64_LO(pFromStruct->m_nnByteCnt));
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEpIpCounter_Validate(sbZfKaEpIpCounter_t *pZf) {
  uint64 nnByteCntMax = COMPILER_64_INIT(0x7, 0xFFFFFFFF);
  if (pZf->m_nPktCnt > 0x1fffffff) return 0;
  if (COMPILER_64_GT(pZf->m_nnByteCnt,nnByteCntMax)) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEpIpCounter_SetField(sbZfKaEpIpCounter_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_npktcnt") == 0) {
    s->m_nPktCnt = value;
  } else if (SB_STRCMP(name, "m_nnbytecnt") == 0) {
    COMPILER_64_SET(s->m_nnByteCnt,0,value);
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
