/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm9600BwAllocCfgBaseEntryConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600BwAllocCfgBaseEntry_Print(sbZfFabBm9600BwAllocCfgBaseEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600BwAllocCfgBaseEntry:: enc=0x%01x"), (unsigned int)  pFromStruct->m_uEnc));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" numvoqs=0x%02x"), (unsigned int)  pFromStruct->m_uNumVoqs));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" basevoq=0x%04x"), (unsigned int)  pFromStruct->m_uBaseVoq));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfFabBm9600BwAllocCfgBaseEntry_SPrint(sbZfFabBm9600BwAllocCfgBaseEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600BwAllocCfgBaseEntry:: enc=0x%01x", (unsigned int)  pFromStruct->m_uEnc);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," numvoqs=0x%02x", (unsigned int)  pFromStruct->m_uNumVoqs);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," basevoq=0x%04x", (unsigned int)  pFromStruct->m_uBaseVoq);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600BwAllocCfgBaseEntry_Validate(sbZfFabBm9600BwAllocCfgBaseEntry_t *pZf) {

  if (pZf->m_uEnc > 0x3) return 0;
  if (pZf->m_uNumVoqs > 0x1f) return 0;
  if (pZf->m_uBaseVoq > 0xffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600BwAllocCfgBaseEntry_SetField(sbZfFabBm9600BwAllocCfgBaseEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_uenc") == 0) {
    s->m_uEnc = value;
  } else if (SB_STRCMP(name, "m_unumvoqs") == 0) {
    s->m_uNumVoqs = value;
  } else if (SB_STRCMP(name, "m_ubasevoq") == 0) {
    s->m_uBaseVoq = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
