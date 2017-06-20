/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm9600BwFetchValidEntryConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600BwFetchValidEntry_Print(sbZfFabBm9600BwFetchValidEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600BwFetchValidEntry:: valid=0x%08x"), (unsigned int)  pFromStruct->m_uValid));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfFabBm9600BwFetchValidEntry_SPrint(sbZfFabBm9600BwFetchValidEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600BwFetchValidEntry:: valid=0x%08x", (unsigned int)  pFromStruct->m_uValid);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600BwFetchValidEntry_Validate(sbZfFabBm9600BwFetchValidEntry_t *pZf) {

  /* pZf->m_uValid implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600BwFetchValidEntry_SetField(sbZfFabBm9600BwFetchValidEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_uvalid") == 0) {
    s->m_uValid = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
