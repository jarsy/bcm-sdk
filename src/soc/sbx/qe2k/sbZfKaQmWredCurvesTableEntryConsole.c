/*
 * $Id: sbZfKaQmWredCurvesTableEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQmWredCurvesTableEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQmWredCurvesTableEntry_Print(sbZfKaQmWredCurvesTableEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQmWredCurvesTableEntry:: tmin=0x%04x"), (unsigned int)  pFromStruct->m_nTmin));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" tmax=0x%04x"), (unsigned int)  pFromStruct->m_nTmax));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" tecn=0x%04x"), (unsigned int)  pFromStruct->m_nTecn));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" scale=0x%01x"), (unsigned int)  pFromStruct->m_nScale));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQmWredCurvesTableEntry:: slope=0x%03x"), (unsigned int)  pFromStruct->m_nSlope));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQmWredCurvesTableEntry_SPrint(sbZfKaQmWredCurvesTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmWredCurvesTableEntry:: tmin=0x%04x", (unsigned int)  pFromStruct->m_nTmin);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," tmax=0x%04x", (unsigned int)  pFromStruct->m_nTmax);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," tecn=0x%04x", (unsigned int)  pFromStruct->m_nTecn);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," scale=0x%01x", (unsigned int)  pFromStruct->m_nScale);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmWredCurvesTableEntry:: slope=0x%03x", (unsigned int)  pFromStruct->m_nSlope);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQmWredCurvesTableEntry_Validate(sbZfKaQmWredCurvesTableEntry_t *pZf) {

  if (pZf->m_nTmin > 0xffff) return 0;
  if (pZf->m_nTmax > 0xffff) return 0;
  if (pZf->m_nTecn > 0xffff) return 0;
  if (pZf->m_nScale > 0xf) return 0;
  if (pZf->m_nSlope > 0xfff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQmWredCurvesTableEntry_SetField(sbZfKaQmWredCurvesTableEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ntmin") == 0) {
    s->m_nTmin = value;
  } else if (SB_STRCMP(name, "m_ntmax") == 0) {
    s->m_nTmax = value;
  } else if (SB_STRCMP(name, "m_ntecn") == 0) {
    s->m_nTecn = value;
  } else if (SB_STRCMP(name, "m_nscale") == 0) {
    s->m_nScale = value;
  } else if (SB_STRCMP(name, "m_nslope") == 0) {
    s->m_nSlope = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
