/*
 * $Id: sbZfKaRbPolicePortQMapEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaRbPolicePortQMapEntryConsole.hx"



/* Print members in struct */
void
sbZfKaRbPolicePortQMapEntry_Print(sbZfKaRbPolicePortQMapEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbPolicePortQMapEntry:: res2=0x%02x"), (unsigned int)  pFromStruct->m_nReserved2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" oddmeter=0x%03x"), (unsigned int)  pFromStruct->m_nOddMeter));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" res1=0x%02x"), (unsigned int)  pFromStruct->m_nReserved1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbPolicePortQMapEntry:: evenmeter=0x%03x"), (unsigned int)  pFromStruct->m_nEvenMeter));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaRbPolicePortQMapEntry_SPrint(sbZfKaRbPolicePortQMapEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbPolicePortQMapEntry:: res2=0x%02x", (unsigned int)  pFromStruct->m_nReserved2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," oddmeter=0x%03x", (unsigned int)  pFromStruct->m_nOddMeter);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," res1=0x%02x", (unsigned int)  pFromStruct->m_nReserved1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbPolicePortQMapEntry:: evenmeter=0x%03x", (unsigned int)  pFromStruct->m_nEvenMeter);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaRbPolicePortQMapEntry_Validate(sbZfKaRbPolicePortQMapEntry_t *pZf) {

  if (pZf->m_nReserved2 > 0x7f) return 0;
  if (pZf->m_nOddMeter > 0x1ff) return 0;
  if (pZf->m_nReserved1 > 0x7f) return 0;
  if (pZf->m_nEvenMeter > 0x1ff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaRbPolicePortQMapEntry_SetField(sbZfKaRbPolicePortQMapEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved2") == 0) {
    s->m_nReserved2 = value;
  } else if (SB_STRCMP(name, "m_noddmeter") == 0) {
    s->m_nOddMeter = value;
  } else if (SB_STRCMP(name, "m_nreserved1") == 0) {
    s->m_nReserved1 = value;
  } else if (SB_STRCMP(name, "m_nevenmeter") == 0) {
    s->m_nEvenMeter = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
