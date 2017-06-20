/*
 * $Id: sbZfKaEpCrTableEntryConsole.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfKaEpCrTableEntryConsole.hx"



/* Print members in struct */
void
sbZfKaEpCrTableEntry_Print(sbZfKaEpCrTableEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEpCrTableEntry:: class15=0x%01x"), (unsigned int)  pFromStruct->m_nClass15));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" class14=0x%01x"), (unsigned int)  pFromStruct->m_nClass14));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" class13=0x%01x"), (unsigned int)  pFromStruct->m_nClass13));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" class12=0x%01x"), (unsigned int)  pFromStruct->m_nClass12));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEpCrTableEntry:: class11=0x%01x"), (unsigned int)  pFromStruct->m_nClass11));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" class10=0x%01x"), (unsigned int)  pFromStruct->m_nClass10));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" class9=0x%01x"), (unsigned int)  pFromStruct->m_nClass9));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" class8=0x%01x"), (unsigned int)  pFromStruct->m_nClass8));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" class7=0x%01x"), (unsigned int)  pFromStruct->m_nClass7));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEpCrTableEntry:: class6=0x%01x"), (unsigned int)  pFromStruct->m_nClass6));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" class5=0x%01x"), (unsigned int)  pFromStruct->m_nClass5));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" class4=0x%01x"), (unsigned int)  pFromStruct->m_nClass4));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" class3=0x%01x"), (unsigned int)  pFromStruct->m_nClass3));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" class2=0x%01x"), (unsigned int)  pFromStruct->m_nClass2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("KaEpCrTableEntry:: class1=0x%01x"), (unsigned int)  pFromStruct->m_nClass1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" class0=0x%01x"), (unsigned int)  pFromStruct->m_nClass0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfKaEpCrTableEntry_SPrint(sbZfKaEpCrTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpCrTableEntry:: class15=0x%01x", (unsigned int)  pFromStruct->m_nClass15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class14=0x%01x", (unsigned int)  pFromStruct->m_nClass14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class13=0x%01x", (unsigned int)  pFromStruct->m_nClass13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class12=0x%01x", (unsigned int)  pFromStruct->m_nClass12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpCrTableEntry:: class11=0x%01x", (unsigned int)  pFromStruct->m_nClass11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class10=0x%01x", (unsigned int)  pFromStruct->m_nClass10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class9=0x%01x", (unsigned int)  pFromStruct->m_nClass9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class8=0x%01x", (unsigned int)  pFromStruct->m_nClass8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class7=0x%01x", (unsigned int)  pFromStruct->m_nClass7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpCrTableEntry:: class6=0x%01x", (unsigned int)  pFromStruct->m_nClass6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class5=0x%01x", (unsigned int)  pFromStruct->m_nClass5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class4=0x%01x", (unsigned int)  pFromStruct->m_nClass4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class3=0x%01x", (unsigned int)  pFromStruct->m_nClass3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class2=0x%01x", (unsigned int)  pFromStruct->m_nClass2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpCrTableEntry:: class1=0x%01x", (unsigned int)  pFromStruct->m_nClass1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," class0=0x%01x", (unsigned int)  pFromStruct->m_nClass0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEpCrTableEntry_Validate(sbZfKaEpCrTableEntry_t *pZf) {

  if (pZf->m_nClass15 > 0xf) return 0;
  if (pZf->m_nClass14 > 0xf) return 0;
  if (pZf->m_nClass13 > 0xf) return 0;
  if (pZf->m_nClass12 > 0xf) return 0;
  if (pZf->m_nClass11 > 0xf) return 0;
  if (pZf->m_nClass10 > 0xf) return 0;
  if (pZf->m_nClass9 > 0xf) return 0;
  if (pZf->m_nClass8 > 0xf) return 0;
  if (pZf->m_nClass7 > 0xf) return 0;
  if (pZf->m_nClass6 > 0xf) return 0;
  if (pZf->m_nClass5 > 0xf) return 0;
  if (pZf->m_nClass4 > 0xf) return 0;
  if (pZf->m_nClass3 > 0xf) return 0;
  if (pZf->m_nClass2 > 0xf) return 0;
  if (pZf->m_nClass1 > 0xf) return 0;
  if (pZf->m_nClass0 > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEpCrTableEntry_SetField(sbZfKaEpCrTableEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nclass15") == 0) {
    s->m_nClass15 = value;
  } else if (SB_STRCMP(name, "m_nclass14") == 0) {
    s->m_nClass14 = value;
  } else if (SB_STRCMP(name, "m_nclass13") == 0) {
    s->m_nClass13 = value;
  } else if (SB_STRCMP(name, "m_nclass12") == 0) {
    s->m_nClass12 = value;
  } else if (SB_STRCMP(name, "m_nclass11") == 0) {
    s->m_nClass11 = value;
  } else if (SB_STRCMP(name, "m_nclass10") == 0) {
    s->m_nClass10 = value;
  } else if (SB_STRCMP(name, "m_nclass9") == 0) {
    s->m_nClass9 = value;
  } else if (SB_STRCMP(name, "m_nclass8") == 0) {
    s->m_nClass8 = value;
  } else if (SB_STRCMP(name, "m_nclass7") == 0) {
    s->m_nClass7 = value;
  } else if (SB_STRCMP(name, "m_nclass6") == 0) {
    s->m_nClass6 = value;
  } else if (SB_STRCMP(name, "m_nclass5") == 0) {
    s->m_nClass5 = value;
  } else if (SB_STRCMP(name, "m_nclass4") == 0) {
    s->m_nClass4 = value;
  } else if (SB_STRCMP(name, "m_nclass3") == 0) {
    s->m_nClass3 = value;
  } else if (SB_STRCMP(name, "m_nclass2") == 0) {
    s->m_nClass2 = value;
  } else if (SB_STRCMP(name, "m_nclass1") == 0) {
    s->m_nClass1 = value;
  } else if (SB_STRCMP(name, "m_nclass0") == 0) {
    s->m_nClass0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
