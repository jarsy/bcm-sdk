/*
 * $Id: sbZfKaQmIngressPortEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQmIngressPortEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQmIngressPortEntry_Print(sbZfKaQmIngressPortEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQmIngressPortEntry:: ingressspi4=0x%01x"), (unsigned int)  pFromStruct->m_nIngressSpi4));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" ingressport=0x%02x"), (unsigned int)  pFromStruct->m_nIngressPort));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQmIngressPortEntry_SPrint(sbZfKaQmIngressPortEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmIngressPortEntry:: ingressspi4=0x%01x", (unsigned int)  pFromStruct->m_nIngressSpi4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ingressport=0x%02x", (unsigned int)  pFromStruct->m_nIngressPort);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQmIngressPortEntry_Validate(sbZfKaQmIngressPortEntry_t *pZf) {

  if (pZf->m_nIngressSpi4 > 0x1) return 0;
  if (pZf->m_nIngressPort > 0x3f) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQmIngressPortEntry_SetField(sbZfKaQmIngressPortEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ningressspi4") == 0) {
    s->m_nIngressSpi4 = value;
  } else if (SB_STRCMP(name, "m_ningressport") == 0) {
    s->m_nIngressPort = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
