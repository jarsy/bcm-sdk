/*
 * $Id: sbZfFabBm9600BwR1Wct2BEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm9600BwR1Wct2BEntryConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600BwR1Wct2BEntry_Print(sbZfFabBm9600BwR1Wct2BEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600BwR1Wct2BEntry:: tecndp2=0x%04x"), (unsigned int)  pFromStruct->m_uTEcnDp2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" scaledp2=0x%01x"), (unsigned int)  pFromStruct->m_uScaleDp2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" slopedp2=0x%03x"), (unsigned int)  pFromStruct->m_uSlopeDp2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfFabBm9600BwR1Wct2BEntry_SPrint(sbZfFabBm9600BwR1Wct2BEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600BwR1Wct2BEntry:: tecndp2=0x%04x", (unsigned int)  pFromStruct->m_uTEcnDp2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," scaledp2=0x%01x", (unsigned int)  pFromStruct->m_uScaleDp2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," slopedp2=0x%03x", (unsigned int)  pFromStruct->m_uSlopeDp2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600BwR1Wct2BEntry_Validate(sbZfFabBm9600BwR1Wct2BEntry_t *pZf) {

  if (pZf->m_uTEcnDp2 > 0xffff) return 0;
  if (pZf->m_uScaleDp2 > 0xf) return 0;
  if (pZf->m_uSlopeDp2 > 0xfff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600BwR1Wct2BEntry_SetField(sbZfFabBm9600BwR1Wct2BEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_utecndp2") == 0) {
    s->m_uTEcnDp2 = value;
  } else if (SB_STRCMP(name, "m_uscaledp2") == 0) {
    s->m_uScaleDp2 = value;
  } else if (SB_STRCMP(name, "m_uslopedp2") == 0) {
    s->m_uSlopeDp2 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
