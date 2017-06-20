/*
 * $Id: sbZfKaRbClassDefaultQEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaRbClassDefaultQEntryConsole.hx"



/* Print members in struct */
void
sbZfKaRbClassDefaultQEntry_Print(sbZfKaRbClassDefaultQEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassDefaultQEntry:: spare1=0x%04x"), (unsigned int)  pFromStruct->m_nSpare1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" defaultdp=0x%01x"), (unsigned int)  pFromStruct->m_nDefaultDp));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" spare0=0x%01x"), (unsigned int)  pFromStruct->m_nSpare0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassDefaultQEntry:: defaultq=0x%04x"), (unsigned int)  pFromStruct->m_nDefaultQ));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaRbClassDefaultQEntry_SPrint(sbZfKaRbClassDefaultQEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassDefaultQEntry:: spare1=0x%04x", (unsigned int)  pFromStruct->m_nSpare1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," defaultdp=0x%01x", (unsigned int)  pFromStruct->m_nDefaultDp);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spare0=0x%01x", (unsigned int)  pFromStruct->m_nSpare0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassDefaultQEntry:: defaultq=0x%04x", (unsigned int)  pFromStruct->m_nDefaultQ);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaRbClassDefaultQEntry_Validate(sbZfKaRbClassDefaultQEntry_t *pZf) {

  if (pZf->m_nSpare1 > 0x3fff) return 0;
  if (pZf->m_nDefaultDp > 0x3) return 0;
  if (pZf->m_nSpare0 > 0x3) return 0;
  if (pZf->m_nDefaultQ > 0x3fff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaRbClassDefaultQEntry_SetField(sbZfKaRbClassDefaultQEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nspare1") == 0) {
    s->m_nSpare1 = value;
  } else if (SB_STRCMP(name, "m_ndefaultdp") == 0) {
    s->m_nDefaultDp = value;
  } else if (SB_STRCMP(name, "m_nspare0") == 0) {
    s->m_nSpare0 = value;
  } else if (SB_STRCMP(name, "m_ndefaultq") == 0) {
    s->m_nDefaultQ = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
