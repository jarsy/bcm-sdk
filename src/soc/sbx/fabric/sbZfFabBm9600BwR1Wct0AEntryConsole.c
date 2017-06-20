/*
 * $Id: sbZfFabBm9600BwR1Wct0AEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm9600BwR1Wct0AEntryConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600BwR1Wct0AEntry_Print(sbZfFabBm9600BwR1Wct0AEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600BwR1Wct0AEntry:: tmindp0=0x%04x"), (unsigned int)  pFromStruct->m_uTMinDp0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" tmaxdp0=0x%04x"), (unsigned int)  pFromStruct->m_uTMaxDp0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfFabBm9600BwR1Wct0AEntry_SPrint(sbZfFabBm9600BwR1Wct0AEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600BwR1Wct0AEntry:: tmindp0=0x%04x", (unsigned int)  pFromStruct->m_uTMinDp0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," tmaxdp0=0x%04x", (unsigned int)  pFromStruct->m_uTMaxDp0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600BwR1Wct0AEntry_Validate(sbZfFabBm9600BwR1Wct0AEntry_t *pZf) {

  if (pZf->m_uTMinDp0 > 0xffff) return 0;
  if (pZf->m_uTMaxDp0 > 0xffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600BwR1Wct0AEntry_SetField(sbZfFabBm9600BwR1Wct0AEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_utmindp0") == 0) {
    s->m_uTMinDp0 = value;
  } else if (SB_STRCMP(name, "m_utmaxdp0") == 0) {
    s->m_uTMaxDp0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
