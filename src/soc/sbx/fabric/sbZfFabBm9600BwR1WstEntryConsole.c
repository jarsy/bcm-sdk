/*
 * $Id: sbZfFabBm9600BwR1WstEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm9600BwR1WstEntryConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600BwR1WstEntry_Print(sbZfFabBm9600BwR1WstEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600BwR1WstEntry:: average=0x%08x"), (unsigned int)  pFromStruct->m_uAverage));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfFabBm9600BwR1WstEntry_SPrint(sbZfFabBm9600BwR1WstEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600BwR1WstEntry:: average=0x%08x", (unsigned int)  pFromStruct->m_uAverage);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600BwR1WstEntry_Validate(sbZfFabBm9600BwR1WstEntry_t *pZf) {

  /* pZf->m_uAverage implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600BwR1WstEntry_SetField(sbZfFabBm9600BwR1WstEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_uaverage") == 0) {
    s->m_uAverage = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
