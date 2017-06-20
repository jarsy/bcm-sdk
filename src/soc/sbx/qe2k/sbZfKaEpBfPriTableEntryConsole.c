/*
 * $Id: sbZfKaEpBfPriTableEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include "sbZfKaEpBfPriTableAddr.hx"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEpBfPriTableEntryConsole.hx"



/* Print members in struct */
void
sbZfKaEpBfPriTableEntry_Print(sbZfKaEpBfPriTableEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEpBfPriTableEntry:: pri7=0x%01x"), (unsigned int)  pFromStruct->m_nPri7));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri6=0x%01x"), (unsigned int)  pFromStruct->m_nPri6));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri5=0x%01x"), (unsigned int)  pFromStruct->m_nPri5));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri4=0x%01x"), (unsigned int)  pFromStruct->m_nPri4));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri3=0x%01x"), (unsigned int)  pFromStruct->m_nPri3));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri2=0x%01x"), (unsigned int)  pFromStruct->m_nPri2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEpBfPriTableEntry:: pri1=0x%01x"), (unsigned int)  pFromStruct->m_nPri1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri0=0x%01x"), (unsigned int)  pFromStruct->m_nPri0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaEpBfPriTableEntry_SPrint(sbZfKaEpBfPriTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpBfPriTableEntry:: pri7=0x%01x", (unsigned int)  pFromStruct->m_nPri7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri6=0x%01x", (unsigned int)  pFromStruct->m_nPri6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri5=0x%01x", (unsigned int)  pFromStruct->m_nPri5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri4=0x%01x", (unsigned int)  pFromStruct->m_nPri4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri3=0x%01x", (unsigned int)  pFromStruct->m_nPri3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri2=0x%01x", (unsigned int)  pFromStruct->m_nPri2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpBfPriTableEntry:: pri1=0x%01x", (unsigned int)  pFromStruct->m_nPri1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri0=0x%01x", (unsigned int)  pFromStruct->m_nPri0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEpBfPriTableEntry_Validate(sbZfKaEpBfPriTableEntry_t *pZf) {

  if (pZf->m_nPri7 > 0x7) return 0;
  if (pZf->m_nPri6 > 0x7) return 0;
  if (pZf->m_nPri5 > 0x7) return 0;
  if (pZf->m_nPri4 > 0x7) return 0;
  if (pZf->m_nPri3 > 0x7) return 0;
  if (pZf->m_nPri2 > 0x7) return 0;
  if (pZf->m_nPri1 > 0x7) return 0;
  if (pZf->m_nPri0 > 0x7) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEpBfPriTableEntry_SetField(sbZfKaEpBfPriTableEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_npri7") == 0) {
    s->m_nPri7 = value;
  } else if (SB_STRCMP(name, "m_npri6") == 0) {
    s->m_nPri6 = value;
  } else if (SB_STRCMP(name, "m_npri5") == 0) {
    s->m_nPri5 = value;
  } else if (SB_STRCMP(name, "m_npri4") == 0) {
    s->m_nPri4 = value;
  } else if (SB_STRCMP(name, "m_npri3") == 0) {
    s->m_nPri3 = value;
  } else if (SB_STRCMP(name, "m_npri2") == 0) {
    s->m_nPri2 = value;
  } else if (SB_STRCMP(name, "m_npri1") == 0) {
    s->m_nPri1 = value;
  } else if (SB_STRCMP(name, "m_npri0") == 0) {
    s->m_nPri0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
