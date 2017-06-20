/*
 * $Id: sbZfKaQmFbLineConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaQmFbLineConsole.hx"



/* Print members in struct */
void
sbZfKaQmFbLine_Print(sbZfKaQmFbLine_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQmFbLine:: hec1=0x%02x"), (unsigned int)  pFromStruct->m_nHec1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" hec0=0x%02x"), (unsigned int)  pFromStruct->m_nHec0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" spr=0x%03x"), (unsigned int)  pFromStruct->m_nSpare));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pb05=0x%05x"), (unsigned int)  pFromStruct->m_nPbExtAddr5));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pb04=0x%05x"), (unsigned int)  pFromStruct->m_nPbExtAddr4));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaQmFbLine:: pb03=0x%05x"), (unsigned int)  pFromStruct->m_nPbExtAddr3));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pb02=0x%05x"), (unsigned int)  pFromStruct->m_nPbExtAddr2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pb01=0x%05x"), (unsigned int)  pFromStruct->m_nPbExtAddr1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pb00=0x%05x"), (unsigned int)  pFromStruct->m_nPbExtAddr0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaQmFbLine_SPrint(sbZfKaQmFbLine_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmFbLine:: hec1=0x%02x", (unsigned int)  pFromStruct->m_nHec1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hec0=0x%02x", (unsigned int)  pFromStruct->m_nHec0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spr=0x%03x", (unsigned int)  pFromStruct->m_nSpare);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pb05=0x%05x", (unsigned int)  pFromStruct->m_nPbExtAddr5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pb04=0x%05x", (unsigned int)  pFromStruct->m_nPbExtAddr4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmFbLine:: pb03=0x%05x", (unsigned int)  pFromStruct->m_nPbExtAddr3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pb02=0x%05x", (unsigned int)  pFromStruct->m_nPbExtAddr2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pb01=0x%05x", (unsigned int)  pFromStruct->m_nPbExtAddr1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pb00=0x%05x", (unsigned int)  pFromStruct->m_nPbExtAddr0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQmFbLine_Validate(sbZfKaQmFbLine_t *pZf) {

  if (pZf->m_nHec1 > 0xff) return 0;
  if (pZf->m_nHec0 > 0xff) return 0;
  if (pZf->m_nSpare > 0x3ff) return 0;
  if (pZf->m_nPbExtAddr5 > 0x1ffff) return 0;
  if (pZf->m_nPbExtAddr4 > 0x1ffff) return 0;
  if (pZf->m_nPbExtAddr3 > 0x1ffff) return 0;
  if (pZf->m_nPbExtAddr2 > 0x1ffff) return 0;
  if (pZf->m_nPbExtAddr1 > 0x1ffff) return 0;
  if (pZf->m_nPbExtAddr0 > 0x1ffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQmFbLine_SetField(sbZfKaQmFbLine_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nhec1") == 0) {
    s->m_nHec1 = value;
  } else if (SB_STRCMP(name, "m_nhec0") == 0) {
    s->m_nHec0 = value;
  } else if (SB_STRCMP(name, "m_nspare") == 0) {
    s->m_nSpare = value;
  } else if (SB_STRCMP(name, "m_npbextaddr5") == 0) {
    s->m_nPbExtAddr5 = value;
  } else if (SB_STRCMP(name, "m_npbextaddr4") == 0) {
    s->m_nPbExtAddr4 = value;
  } else if (SB_STRCMP(name, "m_npbextaddr3") == 0) {
    s->m_nPbExtAddr3 = value;
  } else if (SB_STRCMP(name, "m_npbextaddr2") == 0) {
    s->m_nPbExtAddr2 = value;
  } else if (SB_STRCMP(name, "m_npbextaddr1") == 0) {
    s->m_nPbExtAddr1 = value;
  } else if (SB_STRCMP(name, "m_npbextaddr0") == 0) {
    s->m_nPbExtAddr0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
