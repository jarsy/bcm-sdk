/*
 * $Id: sbZfKaQsAgeThreshLutEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQsAgeThreshLutEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQsAgeThreshLutEntry_Print(sbZfKaQsAgeThreshLutEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQsAgeThreshLutEntry:: res=0x%04x"), (unsigned int)  pFromStruct->m_nReserved));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" anemicthresh=0x%02x"), (unsigned int)  pFromStruct->m_nAnemicThresh));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" efthresh=0x%02x"), (unsigned int)  pFromStruct->m_nEfThresh));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQsAgeThreshLutEntry_SPrint(sbZfKaQsAgeThreshLutEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsAgeThreshLutEntry:: res=0x%04x", (unsigned int)  pFromStruct->m_nReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," anemicthresh=0x%02x", (unsigned int)  pFromStruct->m_nAnemicThresh);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," efthresh=0x%02x", (unsigned int)  pFromStruct->m_nEfThresh);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQsAgeThreshLutEntry_Validate(sbZfKaQsAgeThreshLutEntry_t *pZf) {

  if (pZf->m_nReserved > 0xffff) return 0;
  if (pZf->m_nAnemicThresh > 0xff) return 0;
  if (pZf->m_nEfThresh > 0xff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQsAgeThreshLutEntry_SetField(sbZfKaQsAgeThreshLutEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved") == 0) {
    s->m_nReserved = value;
  } else if (SB_STRCMP(name, "m_nanemicthresh") == 0) {
    s->m_nAnemicThresh = value;
  } else if (SB_STRCMP(name, "m_nefthresh") == 0) {
    s->m_nEfThresh = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
