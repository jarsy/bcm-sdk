/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm9600NmPortsetLinkEntryConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600NmPortsetLinkEntry_Print(sbZfFabBm9600NmPortsetLinkEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600NmPortsetLinkEntry:: index=0x%02x"), (unsigned int)  pFromStruct->m_uIndex));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" nxtptr=0x%02x"), (unsigned int)  pFromStruct->m_uNxtPtr));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfFabBm9600NmPortsetLinkEntry_SPrint(sbZfFabBm9600NmPortsetLinkEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600NmPortsetLinkEntry:: index=0x%02x", (unsigned int)  pFromStruct->m_uIndex);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nxtptr=0x%02x", (unsigned int)  pFromStruct->m_uNxtPtr);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600NmPortsetLinkEntry_Validate(sbZfFabBm9600NmPortsetLinkEntry_t *pZf) {

  if (pZf->m_uIndex > 0xff) return 0;
  if (pZf->m_uNxtPtr > 0xff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600NmPortsetLinkEntry_SetField(sbZfFabBm9600NmPortsetLinkEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_uindex") == 0) {
    s->m_uIndex = value;
  } else if (SB_STRCMP(name, "m_unxtptr") == 0) {
    s->m_uNxtPtr = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
