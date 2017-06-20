/*
 * $Id: sbZfKaEgPortRemapEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEgPortRemapEntryConsole.hx"



/* Print members in struct */
void
sbZfKaEgPortRemapEntry_Print(sbZfKaEgPortRemapEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEgPortRemapEntry:: reserved=0x%02x"), (unsigned int)  pFromStruct->m_nReserved));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" mcfifo=0x%01x"), (unsigned int)  pFromStruct->m_nMcFifo));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" fifoenable=0x%01x"), (unsigned int)  pFromStruct->m_nFifoEnable));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" fifonum=0x%02x"), (unsigned int)  pFromStruct->m_nFifoNum));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaEgPortRemapEntry_SPrint(sbZfKaEgPortRemapEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEgPortRemapEntry:: reserved=0x%02x", (unsigned int)  pFromStruct->m_nReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," mcfifo=0x%01x", (unsigned int)  pFromStruct->m_nMcFifo);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," fifoenable=0x%01x", (unsigned int)  pFromStruct->m_nFifoEnable);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," fifonum=0x%02x", (unsigned int)  pFromStruct->m_nFifoNum);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEgPortRemapEntry_Validate(sbZfKaEgPortRemapEntry_t *pZf) {

  if (pZf->m_nReserved > 0x7f) return 0;
  if (pZf->m_nMcFifo > 0x1) return 0;
  if (pZf->m_nFifoEnable > 0x1) return 0;
  if (pZf->m_nFifoNum > 0x7f) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEgPortRemapEntry_SetField(sbZfKaEgPortRemapEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved") == 0) {
    s->m_nReserved = value;
  } else if (SB_STRCMP(name, "m_nmcfifo") == 0) {
    s->m_nMcFifo = value;
  } else if (SB_STRCMP(name, "m_nfifoenable") == 0) {
    s->m_nFifoEnable = value;
  } else if (SB_STRCMP(name, "m_nfifonum") == 0) {
    s->m_nFifoNum = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
