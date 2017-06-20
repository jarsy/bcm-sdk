/*
 * $Id: sbZfKaRbClassPortEnablesEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaRbClassPortEnablesEntryConsole.hx"



/* Print members in struct */
void
sbZfKaRbClassPortEnablesEntry_Print(sbZfKaRbClassPortEnablesEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassPortEnablesEntry:: reserve=0x%03x"), (unsigned int)  pFromStruct->m_nReserve));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" useuser1=0x%01x"), (unsigned int)  pFromStruct->m_nUseUser1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" useuser0=0x%01x"), (unsigned int)  pFromStruct->m_nUseUser0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassPortEnablesEntry:: usevlanpri=0x%01x"), (unsigned int)  pFromStruct->m_nUseVlanPri));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" usehiprivlan=0x%01x"), (unsigned int)  pFromStruct->m_nUseHiPriVlan));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassPortEnablesEntry:: usedmacmatch=0x%01x"), (unsigned int)  pFromStruct->m_nUseDmacMatch));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" uselayer4=0x%01x"), (unsigned int)  pFromStruct->m_nUseLayer4));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" flowhashenb=0x%01x"), (unsigned int)  pFromStruct->m_nFlowHashEnable));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbClassPortEnablesEntry:: usehashcos=0x%04x"), (unsigned int)  pFromStruct->m_nUseHashCos));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaRbClassPortEnablesEntry_SPrint(sbZfKaRbClassPortEnablesEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassPortEnablesEntry:: reserve=0x%03x", (unsigned int)  pFromStruct->m_nReserve);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," useuser1=0x%01x", (unsigned int)  pFromStruct->m_nUseUser1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," useuser0=0x%01x", (unsigned int)  pFromStruct->m_nUseUser0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassPortEnablesEntry:: usevlanpri=0x%01x", (unsigned int)  pFromStruct->m_nUseVlanPri);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," usehiprivlan=0x%01x", (unsigned int)  pFromStruct->m_nUseHiPriVlan);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassPortEnablesEntry:: usedmacmatch=0x%01x", (unsigned int)  pFromStruct->m_nUseDmacMatch);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," uselayer4=0x%01x", (unsigned int)  pFromStruct->m_nUseLayer4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," flowhashenb=0x%01x", (unsigned int)  pFromStruct->m_nFlowHashEnable);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassPortEnablesEntry:: usehashcos=0x%04x", (unsigned int)  pFromStruct->m_nUseHashCos);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaRbClassPortEnablesEntry_Validate(sbZfKaRbClassPortEnablesEntry_t *pZf) {

  if (pZf->m_nReserve > 0x1ff) return 0;
  if (pZf->m_nUseUser1 > 0x1) return 0;
  if (pZf->m_nUseUser0 > 0x1) return 0;
  if (pZf->m_nUseVlanPri > 0x1) return 0;
  if (pZf->m_nUseHiPriVlan > 0x1) return 0;
  if (pZf->m_nUseDmacMatch > 0x1) return 0;
  if (pZf->m_nUseLayer4 > 0x1) return 0;
  if (pZf->m_nFlowHashEnable > 0x1) return 0;
  if (pZf->m_nUseHashCos > 0xffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaRbClassPortEnablesEntry_SetField(sbZfKaRbClassPortEnablesEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserve") == 0) {
    s->m_nReserve = value;
  } else if (SB_STRCMP(name, "m_nuseuser1") == 0) {
    s->m_nUseUser1 = value;
  } else if (SB_STRCMP(name, "m_nuseuser0") == 0) {
    s->m_nUseUser0 = value;
  } else if (SB_STRCMP(name, "m_nusevlanpri") == 0) {
    s->m_nUseVlanPri = value;
  } else if (SB_STRCMP(name, "m_nusehiprivlan") == 0) {
    s->m_nUseHiPriVlan = value;
  } else if (SB_STRCMP(name, "m_nusedmacmatch") == 0) {
    s->m_nUseDmacMatch = value;
  } else if (SB_STRCMP(name, "m_nuselayer4") == 0) {
    s->m_nUseLayer4 = value;
  } else if (SB_STRCMP(name, "m_nflowhashenable") == 0) {
    s->m_nFlowHashEnable = value;
  } else if (SB_STRCMP(name, "m_nusehashcos") == 0) {
    s->m_nUseHashCos = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
