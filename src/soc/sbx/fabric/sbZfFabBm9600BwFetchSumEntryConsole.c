/*
 * $Id: sbZfFabBm9600BwFetchSumEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm9600BwFetchSumEntryConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600BwFetchSumEntry_Print(sbZfFabBm9600BwFetchSumEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600BwFetchSumEntry:: sum=0x%08x"), (unsigned int)  pFromStruct->m_uSum));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfFabBm9600BwFetchSumEntry_SPrint(sbZfFabBm9600BwFetchSumEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600BwFetchSumEntry:: sum=0x%08x", (unsigned int)  pFromStruct->m_uSum);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600BwFetchSumEntry_Validate(sbZfFabBm9600BwFetchSumEntry_t *pZf) {

  /* pZf->m_uSum implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600BwFetchSumEntry_SetField(sbZfFabBm9600BwFetchSumEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_usum") == 0) {
    s->m_uSum = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
