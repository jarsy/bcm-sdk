/*
 * $Id: sbZfKaEgNotTmePortRemapAddrConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEgNotTmePortRemapAddrConsole.hx"



/* Print members in struct */
void
sbZfKaEgNotTmePortRemapAddr_Print(sbZfKaEgNotTmePortRemapAddr_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEgNotTmePortRemapAddr:: txdma=0x%01x"), (unsigned int)  pFromStruct->m_nTxdma));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" ef=0x%01x"), (unsigned int)  pFromStruct->m_nEf));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" qe1k=0x%01x"), (unsigned int)  pFromStruct->m_nQe1k));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" dport=0x%02x"), (unsigned int)  pFromStruct->m_nPort));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaEgNotTmePortRemapAddr_SPrint(sbZfKaEgNotTmePortRemapAddr_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEgNotTmePortRemapAddr:: txdma=0x%01x", (unsigned int)  pFromStruct->m_nTxdma);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ef=0x%01x", (unsigned int)  pFromStruct->m_nEf);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," qe1k=0x%01x", (unsigned int)  pFromStruct->m_nQe1k);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," dport=0x%02x", (unsigned int)  pFromStruct->m_nPort);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEgNotTmePortRemapAddr_Validate(sbZfKaEgNotTmePortRemapAddr_t *pZf) {

  if (pZf->m_nTxdma > 0x1) return 0;
  if (pZf->m_nEf > 0x1) return 0;
  if (pZf->m_nQe1k > 0x1) return 0;
  if (pZf->m_nPort > 0x3f) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEgNotTmePortRemapAddr_SetField(sbZfKaEgNotTmePortRemapAddr_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ntxdma") == 0) {
    s->m_nTxdma = value;
  } else if (SB_STRCMP(name, "m_nef") == 0) {
    s->m_nEf = value;
  } else if (SB_STRCMP(name, "m_nqe1k") == 0) {
    s->m_nQe1k = value;
  } else if (SB_STRCMP(name, "m_nport") == 0) {
    s->m_nPort = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
