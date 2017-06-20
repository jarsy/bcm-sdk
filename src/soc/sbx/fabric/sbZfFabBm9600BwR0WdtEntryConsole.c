/*
 * $Id: sbZfFabBm9600BwR0WdtEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm9600BwR0WdtEntryConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600BwR0WdtEntry_Print(sbZfFabBm9600BwR0WdtEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600BwR0WdtEntry:: template1=0x%02x"), (unsigned int)  pFromStruct->m_uTemplate1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" spare1=0x%01x"), (unsigned int)  pFromStruct->m_uSpare1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" gain1=0x%01x"), (unsigned int)  pFromStruct->m_uGain1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" template0=0x%02x"), (unsigned int)  pFromStruct->m_uTemplate0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm9600BwR0WdtEntry:: spare0=0x%01x"), (unsigned int)  pFromStruct->m_uSpare0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" gain0=0x%01x"), (unsigned int)  pFromStruct->m_uGain0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfFabBm9600BwR0WdtEntry_SPrint(sbZfFabBm9600BwR0WdtEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600BwR0WdtEntry:: template1=0x%02x", (unsigned int)  pFromStruct->m_uTemplate1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spare1=0x%01x", (unsigned int)  pFromStruct->m_uSpare1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," gain1=0x%01x", (unsigned int)  pFromStruct->m_uGain1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," template0=0x%02x", (unsigned int)  pFromStruct->m_uTemplate0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600BwR0WdtEntry:: spare0=0x%01x", (unsigned int)  pFromStruct->m_uSpare0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," gain0=0x%01x", (unsigned int)  pFromStruct->m_uGain0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600BwR0WdtEntry_Validate(sbZfFabBm9600BwR0WdtEntry_t *pZf) {

  if (pZf->m_uTemplate1 > 0xff) return 0;
  if (pZf->m_uSpare1 > 0xf) return 0;
  if (pZf->m_uGain1 > 0xf) return 0;
  if (pZf->m_uTemplate0 > 0xff) return 0;
  if (pZf->m_uSpare0 > 0xf) return 0;
  if (pZf->m_uGain0 > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600BwR0WdtEntry_SetField(sbZfFabBm9600BwR0WdtEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_utemplate1") == 0) {
    s->m_uTemplate1 = value;
  } else if (SB_STRCMP(name, "m_uspare1") == 0) {
    s->m_uSpare1 = value;
  } else if (SB_STRCMP(name, "m_ugain1") == 0) {
    s->m_uGain1 = value;
  } else if (SB_STRCMP(name, "m_utemplate0") == 0) {
    s->m_uTemplate0 = value;
  } else if (SB_STRCMP(name, "m_uspare0") == 0) {
    s->m_uSpare0 = value;
  } else if (SB_STRCMP(name, "m_ugain0") == 0) {
    s->m_uGain0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
