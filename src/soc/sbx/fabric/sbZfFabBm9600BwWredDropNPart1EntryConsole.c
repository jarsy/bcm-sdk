/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm9600BwWredDropNPart1EntryConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600BwWredDropNPart1Entry_Print(sbZfFabBm9600BwWredDropNPart1Entry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600BwWredDropNPart1Entry:: dp1=0x%01x"), (unsigned int)  pFromStruct->m_uDp1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" ecn1=0x%01x"), (unsigned int)  pFromStruct->m_uEcn1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" reserved1=0x%01x"), (unsigned int)  pFromStruct->m_uReserved1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pbde1=0x%03x"), (unsigned int)  pFromStruct->m_uPbDe1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600BwWredDropNPart1Entry:: dp0=0x%01x"), (unsigned int)  pFromStruct->m_uDp0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" ecn0=0x%01x"), (unsigned int)  pFromStruct->m_uEcn0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" reserved0=0x%01x"), (unsigned int)  pFromStruct->m_uReserved0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pbde0=0x%03x"), (unsigned int)  pFromStruct->m_uPbDe0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfFabBm9600BwWredDropNPart1Entry_SPrint(sbZfFabBm9600BwWredDropNPart1Entry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600BwWredDropNPart1Entry:: dp1=0x%01x", (unsigned int)  pFromStruct->m_uDp1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ecn1=0x%01x", (unsigned int)  pFromStruct->m_uEcn1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," reserved1=0x%01x", (unsigned int)  pFromStruct->m_uReserved1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pbde1=0x%03x", (unsigned int)  pFromStruct->m_uPbDe1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600BwWredDropNPart1Entry:: dp0=0x%01x", (unsigned int)  pFromStruct->m_uDp0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ecn0=0x%01x", (unsigned int)  pFromStruct->m_uEcn0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," reserved0=0x%01x", (unsigned int)  pFromStruct->m_uReserved0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pbde0=0x%03x", (unsigned int)  pFromStruct->m_uPbDe0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600BwWredDropNPart1Entry_Validate(sbZfFabBm9600BwWredDropNPart1Entry_t *pZf) {

  if (pZf->m_uDp1 > 0x1) return 0;
  if (pZf->m_uEcn1 > 0x1) return 0;
  if (pZf->m_uReserved1 > 0xf) return 0;
  if (pZf->m_uPbDe1 > 0x3ff) return 0;
  if (pZf->m_uDp0 > 0x1) return 0;
  if (pZf->m_uEcn0 > 0x1) return 0;
  if (pZf->m_uReserved0 > 0xf) return 0;
  if (pZf->m_uPbDe0 > 0x3ff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600BwWredDropNPart1Entry_SetField(sbZfFabBm9600BwWredDropNPart1Entry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_udp1") == 0) {
    s->m_uDp1 = value;
  } else if (SB_STRCMP(name, "m_uecn1") == 0) {
    s->m_uEcn1 = value;
  } else if (SB_STRCMP(name, "m_ureserved1") == 0) {
    s->m_uReserved1 = value;
  } else if (SB_STRCMP(name, "m_upbde1") == 0) {
    s->m_uPbDe1 = value;
  } else if (SB_STRCMP(name, "m_udp0") == 0) {
    s->m_uDp0 = value;
  } else if (SB_STRCMP(name, "m_uecn0") == 0) {
    s->m_uEcn0 = value;
  } else if (SB_STRCMP(name, "m_ureserved0") == 0) {
    s->m_uReserved0 = value;
  } else if (SB_STRCMP(name, "m_upbde0") == 0) {
    s->m_uPbDe0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
