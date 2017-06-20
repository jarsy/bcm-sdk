/*
 * $Id: sbZfKaQmDemandCfgDataEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQmDemandCfgDataEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQmDemandCfgDataEntry_Print(sbZfKaQmDemandCfgDataEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQmDemandCfgDataEntry:: ratedeltamaxids=0x%02x"), (unsigned int)  pFromStruct->m_nRateDeltaMaxIdx));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" qlademandmask=0x%01x"), (unsigned int)  pFromStruct->m_nQlaDemandMask));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQmDemandCfgDataEntry_SPrint(sbZfKaQmDemandCfgDataEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmDemandCfgDataEntry:: ratedeltamaxids=0x%02x", (unsigned int)  pFromStruct->m_nRateDeltaMaxIdx);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," qlademandmask=0x%01x", (unsigned int)  pFromStruct->m_nQlaDemandMask);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQmDemandCfgDataEntry_Validate(sbZfKaQmDemandCfgDataEntry_t *pZf) {

  if (pZf->m_nRateDeltaMaxIdx > 0x3f) return 0;
  if (pZf->m_nQlaDemandMask > 0x1) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQmDemandCfgDataEntry_SetField(sbZfKaQmDemandCfgDataEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nratedeltamaxidx") == 0) {
    s->m_nRateDeltaMaxIdx = value;
  } else if (SB_STRCMP(name, "m_nqlademandmask") == 0) {
    s->m_nQlaDemandMask = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
