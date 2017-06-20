/*
 * $Id: sbZfKaQmWredParamEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQmWredParamEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQmWredParamEntry_Print(sbZfKaQmWredParamEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQmWredParamEntry:: tmaxexceeded2=0x%01x"), (unsigned int)  pFromStruct->m_nTMaxExceeded2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" ecnexceeded2=0x%01x"), (unsigned int)  pFromStruct->m_nEcnExceeded2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pdrop2=0x%03x"), (unsigned int)  pFromStruct->m_nPDrop2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQmWredParamEntry:: tmaxexceeded1=0x%01x"), (unsigned int)  pFromStruct->m_nTMaxExceeded1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" ecnexceeded1=0x%01x"), (unsigned int)  pFromStruct->m_nEcnExceeded1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pdrop1=0x%03x"), (unsigned int)  pFromStruct->m_nPDrop1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQmWredParamEntry:: tmaxexceeded0=0x%01x"), (unsigned int)  pFromStruct->m_nTMaxExceeded0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" ecnexceeded0=0x%01x"), (unsigned int)  pFromStruct->m_nEcnExceeded0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pdrop0=0x%03x"), (unsigned int)  pFromStruct->m_nPDrop0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQmWredParamEntry_SPrint(sbZfKaQmWredParamEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmWredParamEntry:: tmaxexceeded2=0x%01x", (unsigned int)  pFromStruct->m_nTMaxExceeded2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ecnexceeded2=0x%01x", (unsigned int)  pFromStruct->m_nEcnExceeded2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdrop2=0x%03x", (unsigned int)  pFromStruct->m_nPDrop2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmWredParamEntry:: tmaxexceeded1=0x%01x", (unsigned int)  pFromStruct->m_nTMaxExceeded1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ecnexceeded1=0x%01x", (unsigned int)  pFromStruct->m_nEcnExceeded1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdrop1=0x%03x", (unsigned int)  pFromStruct->m_nPDrop1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmWredParamEntry:: tmaxexceeded0=0x%01x", (unsigned int)  pFromStruct->m_nTMaxExceeded0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ecnexceeded0=0x%01x", (unsigned int)  pFromStruct->m_nEcnExceeded0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdrop0=0x%03x", (unsigned int)  pFromStruct->m_nPDrop0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQmWredParamEntry_Validate(sbZfKaQmWredParamEntry_t *pZf) {

  if (pZf->m_nTMaxExceeded2 > 0x1) return 0;
  if (pZf->m_nEcnExceeded2 > 0x1) return 0;
  if (pZf->m_nPDrop2 > 0x3ff) return 0;
  if (pZf->m_nTMaxExceeded1 > 0x1) return 0;
  if (pZf->m_nEcnExceeded1 > 0x1) return 0;
  if (pZf->m_nPDrop1 > 0x3ff) return 0;
  if (pZf->m_nTMaxExceeded0 > 0x1) return 0;
  if (pZf->m_nEcnExceeded0 > 0x1) return 0;
  if (pZf->m_nPDrop0 > 0x3ff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQmWredParamEntry_SetField(sbZfKaQmWredParamEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ntmaxexceeded2") == 0) {
    s->m_nTMaxExceeded2 = value;
  } else if (SB_STRCMP(name, "m_necnexceeded2") == 0) {
    s->m_nEcnExceeded2 = value;
  } else if (SB_STRCMP(name, "m_npdrop2") == 0) {
    s->m_nPDrop2 = value;
  } else if (SB_STRCMP(name, "m_ntmaxexceeded1") == 0) {
    s->m_nTMaxExceeded1 = value;
  } else if (SB_STRCMP(name, "m_necnexceeded1") == 0) {
    s->m_nEcnExceeded1 = value;
  } else if (SB_STRCMP(name, "m_npdrop1") == 0) {
    s->m_nPDrop1 = value;
  } else if (SB_STRCMP(name, "m_ntmaxexceeded0") == 0) {
    s->m_nTMaxExceeded0 = value;
  } else if (SB_STRCMP(name, "m_necnexceeded0") == 0) {
    s->m_nEcnExceeded0 = value;
  } else if (SB_STRCMP(name, "m_npdrop0") == 0) {
    s->m_nPDrop0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
