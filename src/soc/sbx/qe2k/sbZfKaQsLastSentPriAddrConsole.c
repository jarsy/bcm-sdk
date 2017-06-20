/*
 * $Id: sbZfKaQsLastSentPriAddrConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQsLastSentPriAddrConsole.hx"



/* Print members in struct */
void
sbZfKaQsLastSentPriAddr_Print(sbZfKaQsLastSentPriAddr_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsLastSentPriAddr:: res=0x%05x"), (unsigned int)  pFromStruct->m_nReserved));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" mc=0x%01x"), (unsigned int)  pFromStruct->m_nMc));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" node=0x%02x"), (unsigned int)  pFromStruct->m_nNode));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" dport=0x%02x"), (unsigned int)  pFromStruct->m_nPort));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQsLastSentPriAddr_SPrint(sbZfKaQsLastSentPriAddr_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLastSentPriAddr:: res=0x%05x", (unsigned int)  pFromStruct->m_nReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," mc=0x%01x", (unsigned int)  pFromStruct->m_nMc);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," node=0x%02x", (unsigned int)  pFromStruct->m_nNode);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," dport=0x%02x", (unsigned int)  pFromStruct->m_nPort);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQsLastSentPriAddr_Validate(sbZfKaQsLastSentPriAddr_t *pZf) {

  if (pZf->m_nReserved > 0x7ffff) return 0;
  if (pZf->m_nMc > 0x1) return 0;
  if (pZf->m_nNode > 0x1f) return 0;
  if (pZf->m_nPort > 0x7f) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQsLastSentPriAddr_SetField(sbZfKaQsLastSentPriAddr_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved") == 0) {
    s->m_nReserved = value;
  } else if (SB_STRCMP(name, "m_nmc") == 0) {
    s->m_nMc = value;
  } else if (SB_STRCMP(name, "m_nnode") == 0) {
    s->m_nNode = value;
  } else if (SB_STRCMP(name, "m_nport") == 0) {
    s->m_nPort = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
