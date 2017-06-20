/*
 * $Id: sbZfKaEgTmePortRemapAddrConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEgTmePortRemapAddrConsole.hx"



/* Print members in struct */
void
sbZfKaEgTmePortRemapAddr_Print(sbZfKaEgTmePortRemapAddr_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEgTmePortRemapAddr:: node=0x%02x"), (unsigned int)  pFromStruct->m_nNode));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" dport=0x%02x"), (unsigned int)  pFromStruct->m_nPort));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaEgTmePortRemapAddr_SPrint(sbZfKaEgTmePortRemapAddr_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEgTmePortRemapAddr:: node=0x%02x", (unsigned int)  pFromStruct->m_nNode);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," dport=0x%02x", (unsigned int)  pFromStruct->m_nPort);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEgTmePortRemapAddr_Validate(sbZfKaEgTmePortRemapAddr_t *pZf) {

  if (pZf->m_nNode > 0x1f) return 0;
  if (pZf->m_nPort > 0x1f) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEgTmePortRemapAddr_SetField(sbZfKaEgTmePortRemapAddr_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
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
