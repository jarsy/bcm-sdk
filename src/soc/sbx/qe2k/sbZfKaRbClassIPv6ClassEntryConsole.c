/*
 * $Id: sbZfKaRbClassIPv6ClassEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaRbClassIPv6ClassEntryConsole.hx"



/* Print members in struct */
void
sbZfKaRbClassIPv6ClassEntry_Print(sbZfKaRbClassIPv6ClassEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassIPv6ClassEntry:: res1=0x%01x"), (unsigned int)  pFromStruct->m_nReserve1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" class1dp=0x%01x"), (unsigned int)  pFromStruct->m_nClass1Dp));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" class1lsb=0x%01x"), (unsigned int)  pFromStruct->m_nClass1Lsb));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" res0=0x%01x"), (unsigned int)  pFromStruct->m_nReserve0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassIPv6ClassEntry:: class0dp=0x%01x"), (unsigned int)  pFromStruct->m_nClass0Dp));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" class0lsb=0x%01x"), (unsigned int)  pFromStruct->m_nClass0Lsb));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaRbClassIPv6ClassEntry_SPrint(sbZfKaRbClassIPv6ClassEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassIPv6ClassEntry:: res1=0x%01x", (unsigned int)  pFromStruct->m_nReserve1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class1dp=0x%01x", (unsigned int)  pFromStruct->m_nClass1Dp);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class1lsb=0x%01x", (unsigned int)  pFromStruct->m_nClass1Lsb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," res0=0x%01x", (unsigned int)  pFromStruct->m_nReserve0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassIPv6ClassEntry:: class0dp=0x%01x", (unsigned int)  pFromStruct->m_nClass0Dp);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class0lsb=0x%01x", (unsigned int)  pFromStruct->m_nClass0Lsb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaRbClassIPv6ClassEntry_Validate(sbZfKaRbClassIPv6ClassEntry_t *pZf) {

  if (pZf->m_nReserve1 > 0x3) return 0;
  if (pZf->m_nClass1Dp > 0x3) return 0;
  if (pZf->m_nClass1Lsb > 0xf) return 0;
  if (pZf->m_nReserve0 > 0x3) return 0;
  if (pZf->m_nClass0Dp > 0x3) return 0;
  if (pZf->m_nClass0Lsb > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaRbClassIPv6ClassEntry_SetField(sbZfKaRbClassIPv6ClassEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserve1") == 0) {
    s->m_nReserve1 = value;
  } else if (SB_STRCMP(name, "m_nclass1dp") == 0) {
    s->m_nClass1Dp = value;
  } else if (SB_STRCMP(name, "m_nclass1lsb") == 0) {
    s->m_nClass1Lsb = value;
  } else if (SB_STRCMP(name, "m_nreserve0") == 0) {
    s->m_nReserve0 = value;
  } else if (SB_STRCMP(name, "m_nclass0dp") == 0) {
    s->m_nClass0Dp = value;
  } else if (SB_STRCMP(name, "m_nclass0lsb") == 0) {
    s->m_nClass0Lsb = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
