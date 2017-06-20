/*
 * $Id: sbZfFabBm9600BwR0BwpEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm9600BwR0BwpEntryConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600BwR0BwpEntry_Print(sbZfFabBm9600BwR0BwpEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600BwR0BwpEntry:: gamma=0x%02x"), (unsigned int)  pFromStruct->m_uGamma));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" sigma=0x%06x"), (unsigned int)  pFromStruct->m_uSigma));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfFabBm9600BwR0BwpEntry_SPrint(sbZfFabBm9600BwR0BwpEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600BwR0BwpEntry:: gamma=0x%02x", (unsigned int)  pFromStruct->m_uGamma);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sigma=0x%06x", (unsigned int)  pFromStruct->m_uSigma);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600BwR0BwpEntry_Validate(sbZfFabBm9600BwR0BwpEntry_t *pZf) {

  if (pZf->m_uGamma > 0xff) return 0;
  if (pZf->m_uSigma > 0xffffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600BwR0BwpEntry_SetField(sbZfFabBm9600BwR0BwpEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ugamma") == 0) {
    s->m_uGamma = value;
  } else if (SB_STRCMP(name, "m_usigma") == 0) {
    s->m_uSigma = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
