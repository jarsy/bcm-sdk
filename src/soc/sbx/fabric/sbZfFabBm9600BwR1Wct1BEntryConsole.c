/*
 * $Id: sbZfFabBm9600BwR1Wct1BEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm9600BwR1Wct1BEntryConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600BwR1Wct1BEntry_Print(sbZfFabBm9600BwR1Wct1BEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600BwR1Wct1BEntry:: tecndp1=0x%04x"), (unsigned int)  pFromStruct->m_uTEcnDp1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" scaledp1=0x%01x"), (unsigned int)  pFromStruct->m_uScaleDp1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" slopedp1=0x%03x"), (unsigned int)  pFromStruct->m_uSlopeDp1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfFabBm9600BwR1Wct1BEntry_SPrint(sbZfFabBm9600BwR1Wct1BEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600BwR1Wct1BEntry:: tecndp1=0x%04x", (unsigned int)  pFromStruct->m_uTEcnDp1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," scaledp1=0x%01x", (unsigned int)  pFromStruct->m_uScaleDp1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," slopedp1=0x%03x", (unsigned int)  pFromStruct->m_uSlopeDp1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600BwR1Wct1BEntry_Validate(sbZfFabBm9600BwR1Wct1BEntry_t *pZf) {

  if (pZf->m_uTEcnDp1 > 0xffff) return 0;
  if (pZf->m_uScaleDp1 > 0xf) return 0;
  if (pZf->m_uSlopeDp1 > 0xfff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600BwR1Wct1BEntry_SetField(sbZfFabBm9600BwR1Wct1BEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_utecndp1") == 0) {
    s->m_uTEcnDp1 = value;
  } else if (SB_STRCMP(name, "m_uscaledp1") == 0) {
    s->m_uScaleDp1 = value;
  } else if (SB_STRCMP(name, "m_uslopedp1") == 0) {
    s->m_uSlopeDp1 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
