/*
 * $Id: sbZfKaRbClassIPv4TosEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaRbClassIPv4TosEntryConsole.hx"



/* Print members in struct */
void
sbZfKaRbClassIPv4TosEntry_Print(sbZfKaRbClassIPv4TosEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassIPv4TosEntry:: res1=0x%01x"), (unsigned int)  pFromStruct->m_nReserve1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" tos1dp=0x%01x"), (unsigned int)  pFromStruct->m_nTos1Dp));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" tos1lsb=0x%01x"), (unsigned int)  pFromStruct->m_nTos1Lsb));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" res0=0x%01x"), (unsigned int)  pFromStruct->m_nReserve0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" tos0dp=0x%01x"), (unsigned int)  pFromStruct->m_nTos0Dp));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassIPv4TosEntry:: tos0lsb=0x%01x"), (unsigned int)  pFromStruct->m_nTos0Lsb));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaRbClassIPv4TosEntry_SPrint(sbZfKaRbClassIPv4TosEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassIPv4TosEntry:: res1=0x%01x", (unsigned int)  pFromStruct->m_nReserve1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," tos1dp=0x%01x", (unsigned int)  pFromStruct->m_nTos1Dp);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," tos1lsb=0x%01x", (unsigned int)  pFromStruct->m_nTos1Lsb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," res0=0x%01x", (unsigned int)  pFromStruct->m_nReserve0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," tos0dp=0x%01x", (unsigned int)  pFromStruct->m_nTos0Dp);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassIPv4TosEntry:: tos0lsb=0x%01x", (unsigned int)  pFromStruct->m_nTos0Lsb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaRbClassIPv4TosEntry_Validate(sbZfKaRbClassIPv4TosEntry_t *pZf) {

  if (pZf->m_nReserve1 > 0x3) return 0;
  if (pZf->m_nTos1Dp > 0x3) return 0;
  if (pZf->m_nTos1Lsb > 0xf) return 0;
  if (pZf->m_nReserve0 > 0x3) return 0;
  if (pZf->m_nTos0Dp > 0x3) return 0;
  if (pZf->m_nTos0Lsb > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaRbClassIPv4TosEntry_SetField(sbZfKaRbClassIPv4TosEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserve1") == 0) {
    s->m_nReserve1 = value;
  } else if (SB_STRCMP(name, "m_ntos1dp") == 0) {
    s->m_nTos1Dp = value;
  } else if (SB_STRCMP(name, "m_ntos1lsb") == 0) {
    s->m_nTos1Lsb = value;
  } else if (SB_STRCMP(name, "m_nreserve0") == 0) {
    s->m_nReserve0 = value;
  } else if (SB_STRCMP(name, "m_ntos0dp") == 0) {
    s->m_nTos0Dp = value;
  } else if (SB_STRCMP(name, "m_ntos0lsb") == 0) {
    s->m_nTos0Lsb = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
