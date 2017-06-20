/*
 * $Id: sbZfFabBm9600XbXcfgRemapEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm9600XbXcfgRemapEntryConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600XbXcfgRemapEntry_Print(sbZfFabBm9600XbXcfgRemapEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600XbXcfgRemapEntry:: xcfgb=0x%02x"), (unsigned int)  pFromStruct->m_uXcfgB));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" xcfga=0x%02x"), (unsigned int)  pFromStruct->m_uXcfgA));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfFabBm9600XbXcfgRemapEntry_SPrint(sbZfFabBm9600XbXcfgRemapEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600XbXcfgRemapEntry:: xcfgb=0x%02x", (unsigned int)  pFromStruct->m_uXcfgB);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," xcfga=0x%02x", (unsigned int)  pFromStruct->m_uXcfgA);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600XbXcfgRemapEntry_Validate(sbZfFabBm9600XbXcfgRemapEntry_t *pZf) {

  if (pZf->m_uXcfgB > 0xff) return 0;
  if (pZf->m_uXcfgA > 0xff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600XbXcfgRemapEntry_SetField(sbZfFabBm9600XbXcfgRemapEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_uxcfgb") == 0) {
    s->m_uXcfgB = value;
  } else if (SB_STRCMP(name, "m_uxcfga") == 0) {
    s->m_uXcfgA = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
