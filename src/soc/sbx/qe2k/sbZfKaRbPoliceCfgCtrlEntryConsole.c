/*
 * $Id: sbZfKaRbPoliceCfgCtrlEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaRbPoliceCfgCtrlEntryConsole.hx"



/* Print members in struct */
void
sbZfKaRbPoliceCfgCtrlEntry_Print(sbZfKaRbPoliceCfgCtrlEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbPoliceCfgCtrlEntry:: res=0x%02x"), (unsigned int)  pFromStruct->m_nReserved));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" enable=0x%01x"), (unsigned int)  pFromStruct->m_nEnable));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" notblind=0x%01x"), (unsigned int)  pFromStruct->m_nNotBlind));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" droponred=0x%01x"), (unsigned int)  pFromStruct->m_nDropOnRed));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaRbPoliceCfgCtrlEntry:: enablemon=0x%01x"), (unsigned int)  pFromStruct->m_nEnableMon));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" moncntid=0x%01x"), (unsigned int)  pFromStruct->m_nMonCntId));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" incrate=0x%05x"), (unsigned int)  pFromStruct->m_nIncRate));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaRbPoliceCfgCtrlEntry_SPrint(sbZfKaRbPoliceCfgCtrlEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbPoliceCfgCtrlEntry:: res=0x%02x", (unsigned int)  pFromStruct->m_nReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable=0x%01x", (unsigned int)  pFromStruct->m_nEnable);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," notblind=0x%01x", (unsigned int)  pFromStruct->m_nNotBlind);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," droponred=0x%01x", (unsigned int)  pFromStruct->m_nDropOnRed);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbPoliceCfgCtrlEntry:: enablemon=0x%01x", (unsigned int)  pFromStruct->m_nEnableMon);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," moncntid=0x%01x", (unsigned int)  pFromStruct->m_nMonCntId);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," incrate=0x%05x", (unsigned int)  pFromStruct->m_nIncRate);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaRbPoliceCfgCtrlEntry_Validate(sbZfKaRbPoliceCfgCtrlEntry_t *pZf) {

  if (pZf->m_nReserved > 0x1f) return 0;
  if (pZf->m_nEnable > 0x1) return 0;
  if (pZf->m_nNotBlind > 0x1) return 0;
  if (pZf->m_nDropOnRed > 0x1) return 0;
  if (pZf->m_nEnableMon > 0x1) return 0;
  if (pZf->m_nMonCntId > 0x7) return 0;
  if (pZf->m_nIncRate > 0xfffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaRbPoliceCfgCtrlEntry_SetField(sbZfKaRbPoliceCfgCtrlEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved") == 0) {
    s->m_nReserved = value;
  } else if (SB_STRCMP(name, "m_nenable") == 0) {
    s->m_nEnable = value;
  } else if (SB_STRCMP(name, "m_nnotblind") == 0) {
    s->m_nNotBlind = value;
  } else if (SB_STRCMP(name, "m_ndroponred") == 0) {
    s->m_nDropOnRed = value;
  } else if (SB_STRCMP(name, "m_nenablemon") == 0) {
    s->m_nEnableMon = value;
  } else if (SB_STRCMP(name, "m_nmoncntid") == 0) {
    s->m_nMonCntId = value;
  } else if (SB_STRCMP(name, "m_nincrate") == 0) {
    s->m_nIncRate = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
