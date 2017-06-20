/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm9600BwWredDropNPart2EntryConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600BwWredDropNPart2Entry_Print(sbZfFabBm9600BwWredDropNPart2Entry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600BwWredDropNPart2Entry:: dp2=0x%01x"), (unsigned int)  pFromStruct->m_uDp2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" ecn2=0x%01x"), (unsigned int)  pFromStruct->m_uEcn2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" reserved2=0x%01x"), (unsigned int)  pFromStruct->m_uReserved2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pbde2=0x%03x"), (unsigned int)  pFromStruct->m_uPbDe2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfFabBm9600BwWredDropNPart2Entry_SPrint(sbZfFabBm9600BwWredDropNPart2Entry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600BwWredDropNPart2Entry:: dp2=0x%01x", (unsigned int)  pFromStruct->m_uDp2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ecn2=0x%01x", (unsigned int)  pFromStruct->m_uEcn2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," reserved2=0x%01x", (unsigned int)  pFromStruct->m_uReserved2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pbde2=0x%03x", (unsigned int)  pFromStruct->m_uPbDe2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600BwWredDropNPart2Entry_Validate(sbZfFabBm9600BwWredDropNPart2Entry_t *pZf) {

  if (pZf->m_uDp2 > 0x1) return 0;
  if (pZf->m_uEcn2 > 0x1) return 0;
  if (pZf->m_uReserved2 > 0xf) return 0;
  if (pZf->m_uPbDe2 > 0x3ff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600BwWredDropNPart2Entry_SetField(sbZfFabBm9600BwWredDropNPart2Entry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_udp2") == 0) {
    s->m_uDp2 = value;
  } else if (SB_STRCMP(name, "m_uecn2") == 0) {
    s->m_uEcn2 = value;
  } else if (SB_STRCMP(name, "m_ureserved2") == 0) {
    s->m_uReserved2 = value;
  } else if (SB_STRCMP(name, "m_upbde2") == 0) {
    s->m_uPbDe2 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
